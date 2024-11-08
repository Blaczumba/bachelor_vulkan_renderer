#include "thread_pool.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

Thread::Thread() {
    worker = std::thread(&Thread::queueLoop, this);
}

Thread::~Thread() {
    if (worker.joinable()) {
        wait();
        queueMutex.lock();
        destroying = true;
        condition.notify_one();
        queueMutex.unlock();
        worker.join();
    }
}

void Thread::queueLoop() {
    while (true) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return !jobQueue.empty() || destroying; });
            if (destroying) {
                break;
            }
            job = std::move(jobQueue.front());
        }

        job();

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            jobQueue.pop();
            condition.notify_one();
        }
    }
}

void Thread::addJob(std::function<void()> function) {
    std::lock_guard<std::mutex> lock(queueMutex);
    jobQueue.push(std::move(function));
    condition.notify_one();
}

void Thread::wait() {
    std::unique_lock<std::mutex> lock(queueMutex);
    condition.wait(lock, [this]() { return jobQueue.empty(); });
}

ThreadPool::ThreadPool(size_t count) {
    threads.clear();
    for (uint32_t i = 0; i < count; i++) {
        threads.push_back(std::make_unique<Thread>());
    }
}

Thread* ThreadPool::getThread(size_t index) {
    return threads[index].get();
}

void ThreadPool::wait() {
    for (auto& thread : threads) {
        thread->wait();
    }
}