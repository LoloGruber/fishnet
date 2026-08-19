#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>
#include <fishnet/Option.hpp>

namespace fishnet::util{
/**
 * @brief Blocking queue implementation.
 * Blocks when no elements are present and take() is called until further elements are added using put(...)
 * @tparam T value type
 */
template<typename T>
class BlockingQueue {
protected:
    mutable std::mutex mutex;
    std::queue<fishnet::Option<T>> queue;
    std::condition_variable waitOnNotEmpty;
    std::condition_variable waitOnNotFull;
    static inline fishnet::Option<T> POISON_PILL = std::nullopt;
public:

    void putPoisonPill() {
        this->put(POISON_PILL);
    }

    const fishnet::Option<T> & getPoisonPill(){
        return POISON_PILL;
    }

    virtual void put(fishnet::Option<T> element){
        {
            std::unique_lock<std::mutex> lock(mutex);
            this->queue.push(std::move(element));
        }
        waitOnNotEmpty.notify_one();
    }

    fishnet::Option<T> take(){
        fishnet::Option<T> val = POISON_PILL;
        {
            std::unique_lock<std::mutex>lock(mutex);
            while(this->queue.empty()){
                waitOnNotEmpty.wait(lock);
            }
            val = queue.front();
            queue.pop();
        }
        waitOnNotFull.notify_one();
        return val;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex);
        return this->queue.size();
    }
};
}