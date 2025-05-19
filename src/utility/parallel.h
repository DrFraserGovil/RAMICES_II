#pragma once
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional> // For std::function
#include <atomic>     // For atomic counter
#include <queue>      // Still need a minimal internal queue for chunks
#include <future>
#include "Log.h"

//Alias for a complex compile-time type. If the function is a void-returning-callable, returns void, else returns std::vector<ReturnType>.
//T_Args are included in case T_LoopBodyCallable is itself a template function with a conditional return type; ridiculous futureproofing, but we're here now.
template<class T_LoopBodyCallable, class... T_Args>
using VoidOrVector =
    std::conditional_t<std::is_same_v<std::invoke_result_t<T_LoopBodyCallable, int, T_Args...>, void>,
                       void,
                       std::vector<std::invoke_result_t<T_LoopBodyCallable, int, T_Args...>>>;


class ParallelPool
{
    private:
        std::vector<std::thread> Workers;

        std::queue<std::function<void()>> TaskQueue;
        std::mutex QueueMutex; // Protects the queue
        std::condition_variable TaskAvailable; //signals a task is ready

        std::atomic<int> TasksRemaining;
        std::mutex SyncMutex;
        std::condition_variable ThreadSynchroniser;

        bool StopWorkers; // the final end condition
        
        // The main loop executed by each dedicated worker thread
        void WorkerMain(int workerID);
        void Dispatch(std::function<void()> task);


        // --- Delete copy/move constructors and assignment operators ---
        ParallelPool(const ParallelPool&) = delete;
        ParallelPool& operator=(const ParallelPool&) = delete;
        ParallelPool(ParallelPool&&) = delete;
        ParallelPool& operator=(ParallelPool&&) = delete;


        template<class LoopBodyCallable, class... Args, class ReturnType>
        std::function<void()> CreateChunkedTask(int start, int end, LoopBodyCallable loopBody, std::vector<ReturnType>* results,std::atomic<int> & WorkersActive,  std::condition_variable& ThreadSynchroniser,	std::mutex& SyncMutex, Args&&... args)
        {
            auto boundFunction = std::bind(
                std::forward<LoopBodyCallable>(loopBody), // The original loop body callable
                std::placeholders::_1,                    // Placeholder for the 'int index' (will be passed later)
                std::forward<Args>(args)...             // Forward the variadic arguments pack
            );
        
            // The chunk lambda now captures the bound callable and necessary state
            //'mutable' might be needed if bound_loop is non-copyable or called multiple times requiring state change (unlikely for standard callables)
            return [start, end, results, &WorkersActive, &ThreadSynchroniser, &SyncMutex,boundFunction]() mutable
            {
                // LOG(WARN) << "Captured WA at" << WorkersActive;
                 for (int i = start; i < end; ++i) 
                 {
                     // Call the bound loop body, passing only the current index 'i'.
                     // std::bind ensures the 'args' are automatically passed internally.
                     if constexpr (!std::is_same_v<ReturnType, void>) 
                     {
                         (*results)[i] = boundFunction(i); // Call bound function and store result
                     }
                     else
                     {
                         boundFunction(i); // Call bound function (void return)
                     }
                 }
        
                 // --- Signal completion of THIS chunk ---
                
            };
        }

    public:

        ParallelPool(size_t nCores);
        ~ParallelPool();

        void Synchronise()
        {
             // Notify in case the main thread was the last one and something is waiting (unlikely here, but good practice)
            std::unique_lock<std::mutex> lock(SyncMutex);
            ThreadSynchroniser.notify_one();
        
        
            // --- Wait for the N-1 submitted chunks to complete ---
            // The main thread waits here until all chunks (including its own) are done (counter is 0)
            ThreadSynchroniser.wait(lock, [&]{
                return TasksRemaining == 0; });
        
            // When the wait finishes, all chunks (worker threads' and main thread's) are complete.
        }

    template<class LoopBodyCallable, class... Args, class ReturnType>
    void InternalFor(int Ntask,LoopBodyCallable loopBody, std::vector<ReturnType> *results,Args&&... args)
    {
        size_t N = Workers.size() + 1; // Total workers = pool size + main thread (1)
        TasksRemaining += N;
        size_t movingStart = 0;
        size_t baseBatchSize = Ntask / N;
        size_t chunkOverflow = Ntask % N;
    
        // --- Create and Submit N-1 chunks to the worker pool ---
        for (size_t k = 0; k < N - 1; ++k) {
            size_t batchSize = baseBatchSize + (k < chunkOverflow ? 1 : 0);
            size_t start = movingStart;
            size_t end = movingStart + batchSize;
    
            

            auto task = CreateChunkedTask(start,end,loopBody, results,TasksRemaining,ThreadSynchroniser,SyncMutex,std::forward<Args>(args)... );
            // LOG(DEBUG) << "Dispatched task chunk " << k << " " << start << "->" <<end;
            // Enqueue this chunk task for a worker thread to pick up
            Dispatch(std::move(task)); // Uses internal queue_mutex and task_available_condition
            movingStart = end; // Update start for the next chunk
        }
    
        // --- Main thread executes the last chunk ---
        size_t mainStart = movingStart;
        size_t mainEnd = Ntask; // The last chunk covers the remaining tasks
        
        // LOG(DEBUG) << "Main handling " << mainStart << "  " << mainEnd;
        // Execute the loop body for the main thread's chunk directly
        for (int i = mainStart; i < mainEnd; ++i) 
        {
            if constexpr (!std::is_same_v<ReturnType, void>) {
                (*results)[i] = loopBody(i, std::forward<Args>(args)...); // Forward args here
            } else {
                loopBody(i, std::forward<Args>(args)...); // Forward args here
            }
        }
          // --- Signal completion of the main thread's chunk ---
          --TasksRemaining; // Decrement the counter for the main thread's chunk
    }

    template<class LoopBodyCallable, class... Args>
    auto For(int Ntask, LoopBodyCallable loopBody, Args&&... args) -> VoidOrVector<LoopBodyCallable,Args...>
    {
        if (Ntask <= 0) return; // Handle empty loop
   
            
        if (TasksRemaining > 0)
        {
            LOG(WARN) << "Beginning a parallel-for loop whilst other asynchronous tasks are running is not advised.\n\tFor loops are blocking and occupy the main thread, so this may degrade performance.\n\tCall Synchronise before launching a Parallel-For.";
        }
      
    
   
        using ReturnType = std::invoke_result_t<LoopBodyCallable, int, Args...>;
        // Calculate static chunk ranges for the N total workers
        if constexpr (std::is_same_v<ReturnType, void>) 
        {
            InternalFor(Ntask,loopBody,nullptr,args...);
            Synchronise();
            return;
        }
        else
        {
            std::vector<ReturnType> output;
            output.reserve(Ntask);
            InternalFor(Ntask,loopBody,&output,args...);
            Synchronise();
            return;
        }
      
       
      
    }



    template<class Callable, class... Args>
    auto Task(Callable func, Args&&... args)
    {

        using ReturnType = std::invoke_result_t<Callable,Args...>;
        
        //set up the `promises' that can be cached in at funciton return to ensure value return and synchronisation
        auto promise_ptr = std::make_shared<std::promise<ReturnType>>(); 
        std::future<ReturnType> future = promise_ptr->get_future();

        //If Workers (i.e. extra threads) == 0, then need to execute the function on the main thread. The promises are a bit futile here because it is executed in sequence, but it allows the code to be generalised to work with arbitrary threads.
        if (Workers.size() == 0)
        {
            if constexpr (std::is_same_v<ReturnType, void>) {
                func(args...);
                promise_ptr->set_value(); 
            }
            else
            {
                ReturnType result = func(args...);
                promise_ptr->set_value(std::move(result)); 
            }
            return future;
        }

        //if workers present, bind the promise to a lambda, and pass it to the dispatcher for asynchronous execution
        auto task = [
            task_promise = promise_ptr,
            bound_func = std::bind(std::forward<Callable>(func), std::forward<Args>(args)...)
        ]() mutable
        {
            //check if void or not (compile time)
            //future<void> doesn't return a value, but can ensure that a single task has been executed, so more elegant than a brute force Synchronise check.
            if constexpr (std::is_same_v<ReturnType, void>) {
                bound_func(); 
                task_promise->set_value();
            }
            else
            {
                ReturnType result = bound_func(); 
                task_promise->set_value(std::move(result));
            }
        };
        TasksRemaining++;
        Dispatch(std::move(task));
        return future;
    };
};