#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

namespace lib::thread {

template<typename... Args>
class Worker {
public:
  using Context = std::tuple<Args...>;
  using Job = std::function<void(Args...)>;

  Worker() = default;

  Worker(Args... args, size_t batch = 8);

  void startWorkingThread(Args... args, size_t batch = 8);

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

template <typename... Args>
Worker<Args...>::Worker(Args... args, size_t batch)
  : _context(std::forward<Args>(args)...), _batch(batch), _stop(false),
    _thread(&Worker::workingThread, this) {}

template <typename... Args>
Worker<Args...>::~Worker() {
  {
    std::lock_guard<std::mutex> lock(_mtx);
    _stop = true;
  }

  _cv.notify_one();
  if (_thread.joinable()) {
    _thread.join();
  }
}

template <typename... Args>
void Worker<Args...>::startWorkingThread(Args... args, size_t batch) {
  _context = Context{std::forward<Args>(args)...};
  if (!_thread.joinable()) {
    _batch = batch;
    _stop = false;
    _thread = std::thread(&Worker::workingThread, this);
  }
}

template <typename... Args>
void Worker<Args...>::workingThread() {
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
      std::apply(task, _context);
    }
  }

  while (!_tasks.empty()) {
    std::apply(_tasks.front(), _context);
    _tasks.pop();
  }
}

template <typename... Args>
void Worker<Args...>::addJob(Job&& job) {
  std::lock_guard<std::mutex> lock(_mtx);
  _tasks.push(std::move(job));
  if (_tasks.size() >= _batch) {
      _cv.notify_one();
  }
}

}  // lib::thread
