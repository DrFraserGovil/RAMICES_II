#pragma once
#include "../catch_amalgamated.hpp" 
#include "../../utility/parallel.h"
#include "../../utility/Timer.h"
void createAndDestroy(int n)
{
	auto P = ParallelPool(n);
}
TEST_CASE("Parallel Creation", "[parallel]")
{
	REQUIRE_NOTHROW(createAndDestroy(1));
	REQUIRE_NOTHROW(createAndDestroy(10));
	REQUIRE_THROWS(createAndDestroy(0));
	REQUIRE_THROWS(createAndDestroy(-1));
	REQUIRE_THROWS(createAndDestroy(1000));
	REQUIRE_THROWS(createAndDestroy(301)); //the maximum allowed no. of threads currently hardcoded in
}

void incrementFunction(int i,std::atomic<int>* counter)
{
	(*counter)++;
}

bool FunctionReferenceCapture(int nIncrements,int nWorkers)
{
	auto Parallel = ParallelPool(nWorkers);
	std::atomic<int> counter =0;
	Parallel.For(nIncrements,incrementFunction,&counter);
	return counter == nIncrements;
}

int basicFunction(int i,int fac)
{
	return fac*i;
}
bool ReturnsValuesInVector(int vecsize,int nWorkers)
{
	int fac = 3;
	auto Parallel = ParallelPool(nWorkers);
	std::vector<int> output = Parallel.For(vecsize,basicFunction,fac);
	for (int i = 0; i < vecsize; ++i)
	{
		if (output[i] != i*fac)
		{
			return false;
		}
	}
	return true;
}

bool IsReuseStable(int vecsize,int nWorkers)
{
	auto Parallel = ParallelPool(nWorkers);
	for (int fac = 0; fac < 10; ++fac)
	{
		std::vector<int> output = Parallel.For(vecsize,basicFunction,fac);

		for (int i = 0; i < vecsize; ++i)
		{
			if (output[i] != i*fac)
			{
				return false;
			}
		}
	}
	return true;
}




TEST_CASE("Parallel For Loops","[parallel]")
{
	SECTION("Atomic Pointer Capture")
	{
		for (int nworkers = 1; nworkers < 7; ++nworkers)
		{
			for (int nIncrement = 1; nIncrement < 1e7; nIncrement*=10)
			{
				REQUIRE(FunctionReferenceCapture(nIncrement,nworkers));
			}
		}
	}
	SECTION("Return values")
	{
		for (int nworkers = 1; nworkers < 7; ++nworkers)
		{
			for (int vecSize = 1; vecSize < 1e4; vecSize*=10)
			{
				REQUIRE(ReturnsValuesInVector(vecSize,nworkers));
			}
		}
	}

	SECTION("Repeated use of same pool")
	{
		for (int nworkers = 1; nworkers < 7; ++nworkers)
		{
			for (int vecSize = 1; vecSize < 1e4; vecSize*=10)
			{
				REQUIRE(IsReuseStable(vecSize,nworkers));
			}
		}
	}

	SECTION("Size edge cases")
	{
		for (int n = 1; n < 5; ++n)
		{
			std::atomic<int> counter =0;
			auto P = ParallelPool(n);
			
			SECTION("Void edge cases")
			{
				P.For(0,incrementFunction,&counter);
				bool noChangeWhenNoSteps = counter==0;
				REQUIRE(noChangeWhenNoSteps);
				P.For(1,incrementFunction,&counter);
				bool mainThreadOnly = counter == 1;
				REQUIRE(mainThreadOnly);
			}
			SECTION("Return edge cases")
			{
				for (int r = 0; r < 10; ++r)
				{
					std::vector<int> output = P.For(r,basicFunction,1);
					CHECK(output.size()==r);
				}
			}
		}
	}
}


void basicTask(int &val)
{
	val*=2;
}
int returnTask(int a,int b)
{
	return a*b;
}

TEST_CASE("Task execution","[parallel]")
{
	SECTION("Void-Async Tasks")
	{
		for (int n = 1; n < 5; ++n)
		{
			ParallelPool P(n);
			int value1 = 1;
			int value2 = 2;
			int value3 = 3;
			auto a = P.Task(basicTask,std::ref(value1));
			auto b = P.Task(basicTask,std::ref(value2));
			auto c = P.Task(basicTask,std::ref(value3));
			a.get();
			REQUIRE(value1 == 2);
			b.get();
			REQUIRE(value2 == 4);
			c.get();
			REQUIRE(value3==6);
		}
	}
	SECTION("Return-value Async")
	{
		for (int n = 1; n < 5; ++n)
		{
			ParallelPool P(n);
			int value1 = 1;
			int value2 = 2;
			int value3 = 3;
			auto a = P.Task(returnTask,(value1),value1);
			auto b = P.Task(returnTask,(value2),value2);
			auto c = P.Task(returnTask,(value3),value3);
			
			REQUIRE(a.get() == 1);
			REQUIRE(b.get() == 4);
			REQUIRE(c.get() == 9);
		}
	}
}

///These are written by Gemini, so might be terrible. 

// --- New Callable Examples ---

// Task returning a value
int multiply(int a, int b) {
    return a * b;
}

// Task with const reference and copy
std::string combineStrings(const std::string& s1, std::string s2) {
    return s1 + s2;
}

// Task with unique_ptr (move semantics)
std::unique_ptr<int> squareUniquePtr(std::unique_ptr<int> p) {
    if (p) {
        *p = (*p) * (*p);
    }
    return p;
}

// // --- New TEST_CASEs / Sections ---

TEST_CASE("Task Return Values and Argument Handling", "[parallel][task]")
{
    ParallelPool P(4); // Use a few workers for these tests

    SECTION("Task returning values")
    {
        auto fut_mult = P.Task(multiply, 5, 7);
        auto fut_sum = P.Task([](double x, double y){ return x + y; }, 10.5, 3.2);

        REQUIRE(fut_mult.get() == 35);
        REQUIRE(fut_sum.get() == Catch::Approx(13.7)); // Use Approx for doubles
    }

    SECTION("Task with diverse argument types")
    {
        std::string prefix = "Result: ";
        std::string suffix = "Success!";
        auto fut_str = P.Task(combineStrings, std::cref(prefix), suffix); // std::cref for const&
        REQUIRE(fut_str.get() == "Result: Success!");

      
    }

    SECTION("Individual future::get() blocking")
    {
        std::atomic<bool> task1_started = false;
        std::atomic<bool> task1_finished = false;
        std::atomic<bool> task2_finished = false;

        auto long_task = [&](int id) {
            if (id == 1) task1_started = true;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            if (id == 1) task1_finished = true;
            else task2_finished = true;
            return id;
        };

        auto f1 = P.Task(long_task, 1);
        auto f2 = P.Task(long_task, 2);

        // Give task1 a chance to start, but not necessarily finish
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        REQUIRE(task1_started == true);
        REQUIRE(task1_finished == false); // Should not be finished yet

        REQUIRE(f1.get() == 1); // This call should block until task 1 is done
        REQUIRE(task1_finished == true); // Now task1 must be finished

        // Task 2 might or might not be done, depending on scheduling
        REQUIRE(f2.get() == 2); // This will block if task 2 isn't finished
        REQUIRE(task2_finished == true); // Now task2 must be finished
    }
}

TEST_CASE("Interleaving For and Task Execution", "[parallel][concurrency]")
{
    ParallelPool P(4);
	P.InterleavingWarning = false;
    std::atomic<int> shared_counter = 0; // For tasks
    int for_loop_size = 1000;

    auto increment_task = [&](int id) {
        std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Simulate some work
        shared_counter += id;
        return id;
    };

    SECTION("Tasks then For, then check tasks")
    {
        auto f1 = P.Task(increment_task, 10);
        auto f2 = P.Task(increment_task, 20);

        // The For loop might use available workers or run on the main thread
        // This implicitly checks if workers are correctly managed between task types
        std::vector<int> for_results = P.For(for_loop_size, [](int i){ return i * 2; });

        // Ensure all initial tasks are complete after For, then check their results
        P.Synchronise(); 
        REQUIRE(f1.get() == 10);
        REQUIRE(f2.get() == 20);
        REQUIRE(shared_counter == 30);
        REQUIRE(for_results.size() == for_loop_size);
        REQUIRE(for_results[5] == 10); // Basic check for For results
    }

    SECTION("For then Tasks, then check tasks")
    {
        // Run a For loop first. It should block until complete.
        std::vector<int> for_results = P.For(for_loop_size, [](int i){ return i + 1; });
        REQUIRE(for_results.size() == for_loop_size); // For loop should be done by now

        // Submit tasks after For is complete
        auto f3 = P.Task(increment_task, 100);
        auto f4 = P.Task(increment_task, 200);

        P.Synchronise(); // Synchronize again to ensure tasks are done
        REQUIRE(f3.get() == 100);
        REQUIRE(f4.get() == 200);
        REQUIRE(shared_counter == 300); // 100 + 200
    }
}

TEST_CASE("ParallelPool Destructor Behavior", "[parallel][destructor]")
{
    SECTION("Destructor waits for pending tasks")
    {
        std::atomic<bool> task_started = false;
        std::atomic<bool> task_finished = false;
        
        { // Start a scope for the ParallelPool object
            ParallelPool P(2); 
            auto long_running_task = [&]() {
				task_started = true;
                std::this_thread::sleep_for(std::chrono::milliseconds(150)); // Simulate significant work
                task_finished = true;
            };

            auto f = P.Task(long_running_task); // Submit the task

            // Give the task a moment to definitely start on the worker thread
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            REQUIRE(task_started == true); 
            REQUIRE(task_finished == false); // Task should be started but not yet finished

        } // P goes out of scope here. Its destructor *must* block until long_running_task completes.

        // If the destructor correctly waited, task_finished should now be true
        REQUIRE(task_finished == true); 
    }

    SECTION("Destructor is fast when no tasks are pending")
    {
        auto start_time = std::chrono::high_resolution_clock::now();
        {
            ParallelPool P(4); // Create a pool with workers
            // No tasks submitted or active
        } // P goes out of scope, destructor is called
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // Expect destruction to be very fast (e.g., < 50ms for thread shutdown)
        REQUIRE(duration.count() < 50);
    }
}


bool isPrimeSlow(long long int value)
{
	if (value%2 ==0) return false;
	int maxVal = sqrt(value);
	
	for (int i =3; i<=maxVal; ++i)
	{
		if (value %i ==0)
		{
			return false;
		}
	}
	return true;
}


bool performanceFunction(int i)
{
	long long int val = pow(10,13) + 2*i+1;
	return isPrimeSlow(val);
}

double time(int nWorkers)
{
	Timer T;
	ParallelPool Parallel(nWorkers);
	T.start();
	auto out = Parallel.For(1e4,performanceFunction);
	return T.measure();
}

TEST_CASE("Parallel performance gains","[parallel][performance]")
{
	double timeOneWorker = time(1);
	double timeTwoWorkers = time(2);
	double timeThreeWorkers = time(3);

	CHECK(timeTwoWorkers < timeOneWorker);
	CHECK(timeThreeWorkers < timeTwoWorkers);
}