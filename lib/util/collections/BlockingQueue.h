#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>
#include "Element.h"
template<typename T>
class BlockingQueue {
protected:
    mutable std::mutex mutex;
    std::queue<Element<T>> queue;
    std::condition_variable waitOnNotEmpty;
    std::condition_variable waitOnNotFull;
    static inline  Element<T> POISON_PILL = Element<T>::POISON_PILL;
public:


    void putPoisonPill() {
        this->put(POISON_PILL);
    }

    Element<T> getPoisonPill(){
        return POISON_PILL;
    }



    virtual void put(Element<T> element){
        {
            std::unique_lock<std::mutex> lock(mutex);
            this->queue.push(Element{element});
        }
        waitOnNotEmpty.notify_one();
    }

    void put(T element) {
        put(Element{std::move(element)});
    }

    Element<T> take(){
        Element<T> val = POISON_PILL;
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