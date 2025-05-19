#include "parallel.h"

ParallelPool::ParallelPool(size_t nCores)
{
	if (nCores == 0) nCores = 1;
	StopWorkers = false;
	int nWorkers = nCores -1;
	Workers.reserve(nWorkers);
	for (int i = 0; i < nWorkers; ++i)
	{
		Workers.emplace_back(&ParallelPool::WorkerMain, this,i);
	}
}
ParallelPool::~ParallelPool() {
	// LOG(ERROR) << "Attempting destructor";
	{
		std::unique_lock<std::mutex> lock(QueueMutex);
		StopWorkers = true;
	}
	TaskAvailable.notify_all(); // Notify all workers to check the stop flag

	for (std::thread& worker : Workers) {
		if (worker.joinable()) {
			worker.join(); // Wait for each worker to finish and exit
		}
	}
}

void ParallelPool::Dispatch(std::function<void()> task) {
	{
		std::unique_lock<std::mutex> lock(QueueMutex);
		TaskQueue.push(std::move(task));
	} // Unlock mutex (by passing scope) before notifying
	TaskAvailable.notify_one(); // Signal one worker
}

void ParallelPool::WorkerMain(int workerID) {
	while (true) {
		std::function<void()> task; // Holder for the task
		{
			// Wait for a task or stop signal
			std::unique_lock<std::mutex> lock(QueueMutex);
			TaskAvailable.wait(lock, [&]{
				return StopWorkers || !TaskQueue.empty();
			});

			// Check for stop signal after waking up
			if (StopWorkers && TaskQueue.empty()) {
				// LOG(DEBUG) << workerID << " is exiting";
				return; // Exit the thread
			}

			// Get the next task from the queue
			task = std::move(TaskQueue.front());
			TaskQueue.pop();

		} // Unlock mutex before executing the task


		// Execute the task (which is a chunk lambda)
		task();
		
		//finish the task
		--TasksRemaining;
		 {
			 std::unique_lock<std::mutex> lock(SyncMutex);
			 ThreadSynchroniser.notify_one();
		 }
	}
}
