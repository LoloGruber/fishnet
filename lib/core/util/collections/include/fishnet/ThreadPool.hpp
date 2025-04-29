#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <fishnet/FunctionalConcepts.hpp>

namespace fishnet::util{
class ThreadPool {
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;
public:
    ThreadPool(size_t numThreads) : stop(false) {
        if(numThreads == 0)
            numThreads = 1;
        for (size_t i = 0; i < numThreads; ++i) {
            threads.emplace_back(
                [this] {
                    while (true) {
                        std::function<void()> task;

                        {
                            std::unique_lock<std::mutex> lock(queueMutex);
                            condition.wait(lock, [this] { return stop || !tasks.empty(); });
                            if (stop && tasks.empty())
                                return;
                            task = std::move(tasks.front());
                            tasks.pop();
                        }

                        task();
                    }
                }
            );
        }
    }

    template<fishnet::util::Task F>
    void submit(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }

    void join() {
        if (stop)
            return;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& thread : threads)
            thread.join();
    }

    ~ThreadPool() {
        join();
    }
};
}