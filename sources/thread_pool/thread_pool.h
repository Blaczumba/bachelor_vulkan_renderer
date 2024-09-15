#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <atomic>
#include <type_traits>

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(std::forward<Args>(args)...))> {
        using return_type = decltype(f(std::forward<Args>(args)...));

        auto task = std::packaged_task<return_type()>(std::bind(f, std::forward<Args>(args)...));

        std::future<return_type> res = task.get_future();

        {
            std::unique_lock<std::mutex> lock(queueMutex);

            if (stop)
                throw std::runtime_error("enqueue on stopped ThreadPool");

            tasks.emplace([&task]() { (task)(); });
        }

        condition.notify_one();

        return res;
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    std::atomic<bool> stop{ false };
};
