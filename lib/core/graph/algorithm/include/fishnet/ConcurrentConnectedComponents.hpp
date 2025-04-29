#pragma once
#include "ConnectedComponents.hpp"
#include <fishnet/BlockingQueue.hpp>
namespace fishnet::graph{
/**
 * @brief Concurrent specialization for computing connected components
 * 
 * @tparam N node type
 * @tparam Hash hasher type on nodes
 * @tparam Equal comparator type on nodes
 */
template<typename N, util::HashFunction<N> Hash= std::hash<N>,NodeBiPredicate<N> Equal = std::equal_to<N>>
class ConcurrentConnectedComponents: public ConnectedComponents<N,Hash,Equal>
{
private:
    using QueuePtr = std::shared_ptr<fishnet::util::BlockingQueue<std::pair<int,std::vector<N>>>>;
    QueuePtr queue;
protected:
    void handleClose() override {
        this->queue->put(std::make_pair(this->index,this->components.back())); // add connected component to blocking queue on close
    }
public:
    ConcurrentConnectedComponents(QueuePtr q):ConnectedComponents<N,Hash,Equal>(),queue(q){};
    ~ConcurrentConnectedComponents()=default;
};
}