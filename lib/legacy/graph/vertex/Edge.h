#ifndef GRAPH_EDGE_H
#define GRAPH_EDGE_H
#include "NetworkConcepts.h"
#include <optional>
namespace graph{

template<typename N, typename A, typename N_Hash = std::hash<N>, typename N_Equal = std::equal_to<N>> requires Node<N> && Annotation<A>
class Edge 
{
protected:
    N  from;
    N  to;
    std::optional<A> weight;
    N_Equal equalityFunction = N_Equal();
    
public:
    Edge(const N & from, const N & to):from(from),to(to),weight(std::nullopt) {};
    Edge(const N & from, const N & to,const A & weight): from(from),to(to),weight(std::optional<A>{weight}){};
    Edge(const N & from, const N & to, const std::optional<A> & optWeight): from(from),to(to),weight(optWeight){};
    Edge(const Edge<N,A,N_Hash,N_Equal> & edge): from(edge.getFrom()), to(edge.getTo()), weight(edge.getWeight()) {};

    const N & getFrom() const{
        return this->from;
    }

    const N & getTo() const {
        return this->to;
    }

    const std::optional<A> & getWeight() const {
        return this->weight;
    }


    virtual bool operator==(const Edge<N,A,N_Hash,N_Equal> & other) const = 0;
    virtual ~Edge() = default;


};
}
#endif