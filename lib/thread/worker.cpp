#include "worker.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <queue>

namespace lib::thread {

Worker::Worker(size_t batch) : _thread(&Worker::workingThread, this), _stop(false), _batch(batch) {}

Worker::~Worker() {
  {
    std::lock_guard<std::mutex> lock(_mtx);
    _stop = true;
  }
  _cv.notify_one();
  _thread.join();
}

void Worker::workingThread() {
  std::vector<std::function<void()>> tasksToProcess(_batch); // TODO: Change to inline vector
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

    for (std::function<void()>& task : tasksToProcess) {
      task();
    }
  }

  while (!_tasks.empty()) {
    _tasks.front()();
    _tasks.pop();
  }
}

void Worker::addJob(std::function<void()>&& job) {
  std::lock_guard<std::mutex> lock(_mtx);
  _tasks.push(std::move(job));
  if (_tasks.size() >= _batch) {
    _cv.notify_one();
  }
}

}  // namespace lib::thread
