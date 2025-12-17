#ifndef GRAPH_GRAPH_H
#define GRAPH_GRAPH_H
#include "Edge.h"
#include <vector>
#include <memory>
#include "NetworkConcepts.h"



namespace graph{
template<Node N, Annotation A,HashFunction<N> Hash=std::hash<N>,NodeBiPredicate<N> Equal = std::equal_to<N>>
class Graph
{
protected:
    Equal equalityFunction = Equal();
    Hash hashFunction = Hash();
public:
    using value_type =  std::decay<N>;
    using annotation_type =  std::decay<A>;


    Hash hash_function() const {
        return Hash();
    }

    Equal equality_function() const {
        return Equal();
    }


    virtual void addNode(N & node) = 0;
    virtual void addNode(N && node) = 0;
    virtual bool containsNode(const N & node) const = 0;
    virtual void removeNode(const N & node) = 0;

    virtual void addEdge( N & from,  N & to) = 0;
    virtual void addEdge(N && from, N && to) = 0;
    virtual bool containsEdge(const N & from, const N & to) const= 0;
    bool containsEdge(const Edge<N,A> & edge) const{
        return containsEdge(edge.getFrom(),edge.getTo());
    };
    bool containsEdge(const std::unique_ptr<Edge<N,A>> & edge) const {
        return containsEdge(edge->getFrom(),edge->getTo());
    }

    virtual void removeEdge(const N & from, const N & to) = 0;
    inline virtual std::unique_ptr<Edge<N,A>> makeEdge(const N & from, const N & to) const = 0;

    virtual const std::vector<N> getNodes() const =0;
    virtual const std::vector<std::unique_ptr<Edge<N,A>>> getEdges() const =0;
    virtual const std::vector<N> getNeighbours(const N & node) const =0;
    virtual const std::vector<N> getReachableFrom(const N & node) const = 0;
    virtual const std::vector<std::unique_ptr<Edge<N,A>>> getInboundEdges(const N & node) const = 0;
    virtual const std::vector<std::unique_ptr<Edge<N,A>>> getOutboundEdges(const N & node) const = 0; 


    virtual void clear() = 0;
    virtual ~Graph() = default;
};
}




#endif