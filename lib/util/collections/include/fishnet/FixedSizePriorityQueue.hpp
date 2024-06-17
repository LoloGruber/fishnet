#pragma once 
#include <queue>

namespace fishnet::util {
template<typename T, typename Mapper>
class FixedSizePriorityQueue{
private:
    std::vector<T> queue;// Max-Heap queue
    size_t capacity;
    Mapper mapper;
public:
    FixedSizePriorityQueue(size_t capacity,Mapper comparator):capacity(capacity),mapper(comparator){
        queue.reserve(capacity);
    }

    bool empty() const noexcept {
        return queue.empty();
    }

    void push(T && value){
        if(queue.size() < capacity){
            queue.push_back(std::move(value));
        }else{
            int swapIndex=0;
            auto max = mapper(queue[0]);
            for(int i=1; i < int(capacity);i++){
                auto val = mapper(queue[i]);
                if(val> max){
                    max = val;
                    swapIndex = i;
                }
            }
            if(mapper(value) < max)
                queue[swapIndex] = std::move(value);
        }
    }

    void push(const T & value){
        T copy = value;
        push(std::move(copy));
    }

    auto begin(){
        return queue.begin();
    }

    auto end(){
        return queue.end();
    }
};
}