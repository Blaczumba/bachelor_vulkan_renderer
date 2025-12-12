#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

namespace lib::thread {

template<typename Context>
class Worker {
public:
  using Job = std::function<void(Context)>;

  Worker() = default;

  Worker(const Context& context, size_t batch = 8);

  void startWorkingThread(const Context& context, size_t batch = 8);

  ~Worker();

  void addJob(Job&& job);

private:
  void workingThread();

  Context _context;
  size_t _batch;
  bool _stop;
  std::queue<Job> _tasks;
  std::thread _thread;
  std::mutex _mtx;
  std::condition_variable _cv;
};

template <typename Context>
Worker<Context>::Worker(const Context& context, size_t batch)
  : _context(context), _batch(batch), _stop(false), _thread(&Worker::workingThread, this) {}

template <typename Context>
Worker<Context>::~Worker() {
  {
    std::lock_guard<std::mutex> lock(_mtx);
    _stop = true;
  }

  _cv.notify_one();
  if (_thread.joinable()) {
    _thread.join();
  }
}

template <typename Context>
void Worker<Context>::startWorkingThread(const Context& context, size_t batch) {
  if (_thread.joinable()) {
    return;
  }
  _context = context;
  _batch = batch;
  _stop = false;
  _thread = std::thread(&Worker::workingThread, this);
}

template <typename Context>
void Worker<Context>::workingThread() {
  std::vector<Job> tasksToProcess(_batch);  // TODO: Change to inline vector
  while (true) {
    {
      std::unique_lock<std::mutex> lock(_mtx);
      _cv.wait(lock, [this]() {
        return _tasks.size() > _batch || _stop;
      });

      if (_stop) [[unlikely]] {
        break;
      }

      for (size_t i = 0; i < _batch; i++) {
        tasksToProcess[i] = std::move(_tasks.front());
        _tasks.pop();
      }
    }

    for (Job& task : tasksToProcess) {
      task(_context);
    }
  }

  while (!_tasks.empty()) {
    _tasks.front()(_context);
    _tasks.pop();
  }
}

template <typename Context>
void Worker<Context>::addJob(Job&& job) {
  std::lock_guard<std::mutex> lock(_mtx);
  _tasks.push(std::move(job));
  if (_tasks.size() >= _batch) {
    _cv.notify_one();
  }
}

}  // namespace lib::thread
