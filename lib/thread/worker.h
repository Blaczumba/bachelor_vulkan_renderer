#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

namespace lib::thread {
	
class Worker {
public:
  Worker(size_t batch = 8);

  ~Worker();

  void addJob(std::function<void()>&& job);

private:
  void workingThread();

  std::thread _thread;
  std::mutex _mtx;
  std::condition_variable _cv;
  std::queue<std::function<void()>> _tasks;
  size_t _batch;
  bool _stop;
};

}  // namespace lib::thread
