#pragma once
#include "ConnectedComponents.h"
#include "BlockingQueue.h"
namespace fishnet::graph{

template<typename N, util::HashFunction<N> Hash= std::hash<N>,NodeBiPredicate<N> Equal = std::equal_to<N>>
class ConcurrentConnectedComponents: public ConnectedComponents<N,Hash,Equal>
{
private:
    std::shared_ptr<BlockingQueue<std::pair<int,std::vector<N>>>> queue;
protected:
    void handleClose() override {
        this->queue->put(std::make_pair(this->index,this->components.back()));
    }
public:
    ConcurrentConnectedComponents(std::shared_ptr<BlockingQueue<std::pair<int,std::vector<N>>>> q):ConnectedComponents<N,Hash,Equal>(),queue(q){};
    ~ConcurrentConnectedComponents()=default;
};
}