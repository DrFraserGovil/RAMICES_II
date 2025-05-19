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
#include "referenceTesters.h"
//Alias for a complex compile-time type. If the function is a void-returning-callable, returns void, else returns std::vector<ReturnType>.
//T_Args are included in case T_LoopBodyCallable is itself a template function with a conditional return type; ridiculous futureproofing, but we're here now.
template<class T_LoopBodyCallable, class... T_Args>
using VoidOrVector =
    std::conditional_t<std::is_same_v<std::invoke_result_t<T_LoopBodyCallable, int, T_Args...>, void>,
                       void,
                       std::vector<std::invoke_result_t<T_LoopBodyCallable, int, T_Args...>>>;



//Defines an object which spins up a bunch of workers which wait for new asynchronous tasks to be given to them.
//It is primarily designed for the 'For' loop; the generalised 'Task' interface will probably be rarely used.
class ParallelPool
{
    private:

        std::vector<std::thread> Workers;

        std::queue<std::function<void()>> TaskQueue;
        std::mutex QueueMutex; // Protects the queue
        std::condition_variable TaskAvailable; //signals a task is ready

        std::atomic<int> TasksRemaining; //Counts the number of active tasks in the queue. This should rarely, if ever, exceed the no. of Workers. This ensures rapid uptake & high efficiency
        
        std::mutex SyncMutex; //used to allow the pool to synchronise and wait for al other tasks to 
        std::condition_variable ThreadSynchroniser;

        bool StopWorkers; // the final end condition
        
        // The main loop executed by each dedicated worker thread
        void WorkerMain(int workerID);

        //The internal function which adds tasks to the queue, increments etc.
        void Dispatch(std::function<void()> task);

        template<class LoopBodyCallable, class FunctionReturnType,class... Args>
        void DispatchChunk(int start, int end, LoopBodyCallable loopBody, std::vector<FunctionReturnType>* results, Args&&... args)
        {
            


            auto boundFunction = std::bind(
                std::forward<LoopBodyCallable>(loopBody), // The original loop body callable
                std::placeholders::_1,                    // Placeholder for the 'int index' (will be passed later)
                std::forward<Args>(args)...             // Forward the variadic arguments pack
            );
        
            // The chunk lambda now captures the bound callable and necessary state
            
            std::function<void()> func =[start, end, results,boundFunction]() mutable
            {
                //this exists solely to prevent compiler warnings that result when a function has void return type and so results is unused. 
                //hopefully it does absolutely nothing aside from tell the compiler to be quiet.
                if constexpr (std::is_same_v<FunctionReturnType, void>) 
                {
                    (void)results;
                }

                // LOG(WARN) << "Captured WA at" << WorkersActive;
                 for (int i = start; i < end; ++i) 
                 {
                     // Call the bound loop body, passing only the current index 'i'.
                     // std::bind ensures the 'args' are automatically passed internally.
                     if constexpr (!std::is_same_v<FunctionReturnType, void>) 
                     {
                         (*results)[i] = boundFunction(i); // Call bound function and store result
                     }
                     else
                     {
                         boundFunction(i); // Call bound function (void return)
                        
                         
                     }
                 }
            };
            Dispatch(std::move(func));
        }

        template<class LoopBodyCallable, class ReturnType,class... Args>
        void InternalFor(int Ntask,LoopBodyCallable loopBody, std::vector<ReturnType> *results,Args&&... args)
        {
            size_t N = Workers.size() + 1; // Total workers = pool size + main thread (1)
            {
                std::unique_lock<std::mutex> lock(SyncMutex);
                TasksRemaining += N;
            }
            // --- Create and Submit N-1 chunks to the worker pool ---
            
            size_t movingStart = 0;
            size_t baseBatchSize = Ntask / N;
            size_t chunkOverflow = Ntask % N;
            for (size_t k = 0; k < N - 1; ++k) {
                size_t batchSize = baseBatchSize + (k < chunkOverflow ? 1 : 0);
                size_t start = movingStart;
                size_t end = movingStart + batchSize;
        
        
                DispatchChunk(start,end,loopBody, results,std::forward<Args>(args)... );
                movingStart = end; 
            }
        
            // --- Main thread executes the last chunk ---
            for (int i = movingStart; i < Ntask; ++i) 
            {
                if constexpr (!std::is_same_v<ReturnType, void>) {
                    (*results)[i] = loopBody(i, std::forward<Args>(args)...); 
                } else {
                    loopBody(i, std::forward<Args>(args)...); 
                }
            }
              // --- Signal completion of the main thread's chunk ---
              --TasksRemaining; // Decrement the counter for the main thread's chunk
        }
    
          // --- Delete copy/move constructors and assignment operators ---
          ParallelPool(const ParallelPool&) = delete;
          ParallelPool& operator=(const ParallelPool&) = delete;
          ParallelPool(ParallelPool&&) = delete;
          ParallelPool& operator=(ParallelPool&&) = delete;
  
    public:
        bool InterleavingWarning;

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

   
    template<class LoopBodyCallable, class... Args>
    auto For(int Ntask, LoopBodyCallable loopBody, Args&&... args) -> VoidOrVector<LoopBodyCallable,Args...>
    {    
        COMPILE_TIME_REFERENCE_CATCHER(1,LoopBodyCallable,Args); //errors generated here are just intellisense not getting the macro!

        if (TasksRemaining > 0 && InterleavingWarning )
        {
            LOG(WARN) << "Beginning a parallel-for loop whilst other asynchronous tasks are running is not advised.\n\tFor loops are blocking and occupy the main thread, so this may degrade performance.\n\tCall Synchronise before launching a Parallel-For.";
        }
      
    
        //slightly different calls depending on the function return type. If void, execute with a dummy pointer, otherwise create the holder for the return type.
        using ReturnType = std::invoke_result_t<LoopBodyCallable, int, Args...>;
        if constexpr (std::is_same_v<ReturnType, void>) 
        {
            if (Ntask == 0) return;
            InternalFor<LoopBodyCallable,void,Args...>(Ntask,loopBody,nullptr,std::forward<Args>(args)...);
            Synchronise();
            return;
        }
        else
        {
            std::vector<ReturnType> output;
            if (Ntask == 0) return output;
            output.resize(Ntask);
            InternalFor<LoopBodyCallable,ReturnType,Args...>(Ntask,loopBody,&output,std::forward<Args>(args)...);
            Synchronise();
            return output;
        } 
    }


    //a generic aysnchronous executor that allows tasks to be inserted into the queue (when not already running a For loop)
    //Returns std::future objects which allow return value retrieval, or selective synchronisation.
    template<class Callable, class... Args>
    auto Task(Callable func, Args&&... args)
    {
        COMPILE_TIME_REFERENCE_CATCHER(0,Callable,Args);//errors generated here are just intellisense not getting the macro!
        using ReturnType = std::invoke_result_t<Callable,Args...>;
        
        //set up the `promises' that can be cached in at funciton return to ensure value return and synchronisation
        auto promise_ptr = std::make_shared<std::promise<ReturnType>>(); 
        std::future<ReturnType> future = promise_ptr->get_future();

        //If Workers (i.e. extra threads) == 0, then need to execute the function on the main thread. The promises are a bit futile here because it is executed in sequence, but it allows the code to be generalised to work with arbitrary threads.
        if (Workers.size() == 0)
        {
            if constexpr (std::is_same_v<ReturnType, void>) {
                func(std::forward<Args>(args)...);
                promise_ptr->set_value(); 
            }
            else
            {
                ReturnType result = func(std::forward<Args>(args)...);
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
        {
			std::unique_lock<std::mutex> lock(SyncMutex);
            TasksRemaining++;
        }
        Dispatch(std::move(task));
        return future;
    };
};