#ifndef GRAPH_WEIGHTEDGRAPH_H
#define GRAPH_WEIGHTEDGRAPH_H
#include "Graph.h"
namespace graph {
template<Node N, Annotation A, WeightFn<N,A> WeightFunction = EmptyWeightFunction<N,A>,HashFunction<N> Hash=std::hash<N>,NodeBiPredicate<N> Equal = std::equal_to<N> > 
class WeightedGraph: public Graph<N,A,Hash,Equal>{
public:
    std::optional<A> getWeight(const N & from, const N & to) const{
        return this->weightFunction(from,to);
    }
    virtual ~WeightedGraph() = default;
    WeightedGraph(){
        this->weightFunction = WeightFunction();
    }
protected:
    WeightFunction weightFunction;

};



}


#endif