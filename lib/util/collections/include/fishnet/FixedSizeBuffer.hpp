#pragma once 
#include <vector>
#include <fishnet/FunctionalConcepts.hpp>

namespace fishnet::util {
template<typename T, typename C> requires std::totally_ordered<C>
class FixedSizeBuffer{
private:
    std::vector<T> collection;
    std::vector<C> cache;
    size_t capacity;
    std::function<C(const T &)> mapper;
public:
    FixedSizeBuffer(size_t capacity,UnaryFunction<T,C> auto mapper):capacity(capacity),mapper(mapper){
        collection.reserve(capacity);
        cache.reserve(capacity);
    }

    bool empty() const noexcept {
        return collection.empty();
    }

    void push(T && value){
        if(collection.size() < capacity){
            cache.push_back(mapper(value));
            collection.push_back(std::move(value));
        }else{
            int swapIndex=0;
            auto max = cache[0];
            for(int i=1; i < int(capacity);i++){
                auto val = cache[i];
                if(val> max){
                    max = val;
                    swapIndex = i;
                }
            }
            if(mapper(value) < max){
                cache[swapIndex] = mapper(value);
                collection[swapIndex] = std::move(value);
            }
        }
    }

    void push(const T & value){
        T copy = value;
        push(std::move(copy));
    }

    auto begin(){
        return collection.begin();
    }

    auto end(){
        return collection.end();
    }
};
}