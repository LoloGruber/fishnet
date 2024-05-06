#pragma once
#include "BlockingQueue.hpp"
namespace fishnet::util{
template<typename T>
class FiniteBlockingQueue : public BlockingQueue<T>
{
private:
    size_t capacity;
public:

    FiniteBlockingQueue(size_t capacity):BlockingQueue<T>() {
        if  (capacity==0) {
            this->capacity = (size_t) -1;
        }
        this->capacity = capacity;
    }

    FiniteBlockingQueue():BlockingQueue<T>(){
        this->capacity = (size_t) -1;
    }

    void put(Element<T> element)override{
        std::unique_lock<std::mutex> lock(this->mutex);
        while(this->queue.size()>= this->capacity) {
            this->waitOnNotFull.wait(lock);
        }
        this->queue.push(element);
        this->waitOnNotEmpty.notify_one();
    }
};
}

