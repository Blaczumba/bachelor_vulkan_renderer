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

class Thread {
    bool destroying = false;
    std::thread worker;
    std::queue<std::function<void()>> jobQueue;
    std::mutex queueMutex;
    std::condition_variable condition;

    void queueLoop();

public:
    Thread();
    ~Thread();

    void addJob(std::function<void()> function);
    void wait();
};

class ThreadPool {
    std::vector<std::unique_ptr<Thread>> threads;

public:
    ThreadPool(size_t count);
    Thread* getThread(size_t index);
    size_t getNumThreads() const;
    void wait();
};