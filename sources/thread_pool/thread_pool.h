#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <utility>
#include <type_traits>
#include <memory>

class Thread
{
private:
	bool destroying = false;
	std::thread worker;
	std::queue<std::function<void()>> jobQueue;
	std::mutex queueMutex;
	std::condition_variable condition;

	// Loop through all remaining jobs
	void queueLoop()
	{
		while (true)
		{
			std::function<void()> job;
			{
				std::unique_lock<std::mutex> lock(queueMutex);
				condition.wait(lock, [this] { return !jobQueue.empty() || destroying; });
				if (destroying)
				{
					break;
				}
				job = std::move(jobQueue.front());
			}

			job();

			{
				std::lock_guard<std::mutex> lck(queueMutex);
				jobQueue.pop();
				condition.notify_one();
			}
		}
	}

public:
	Thread()
	{
		worker = std::thread(&Thread::queueLoop, this);
	}

	~Thread()
	{
		if (worker.joinable())
		{
			wait();
			queueMutex.lock();
			destroying = true;
			condition.notify_one();
			queueMutex.unlock();
			worker.join();
		}
	}

	// Add a new job to the thread's queue
	void addJob(std::function<void()> function)
	{
		std::lock_guard<std::mutex> lock(queueMutex);
		jobQueue.push(std::move(function));
		condition.notify_one();
	}

	// Wait until all work items have been finished
	void wait()
	{
		std::unique_lock<std::mutex> lock(queueMutex);
		condition.wait(lock, [this]() { return jobQueue.empty(); });
	}
};

class ThreadPool
{
	std::vector<std::unique_ptr<Thread>> threads;
public:

	ThreadPool(size_t count) {
		threads.clear();
		for (uint32_t i = 0; i < count; i++)
		{
			threads.push_back(std::make_unique<Thread>());
		}
	}

	Thread* getThread(size_t index) {
		return threads[index].get();
	}

	// Wait until all threads have finished their work items
	void wait()
	{
		for (auto& thread : threads)
		{
			thread->wait();
		}
	}
};
