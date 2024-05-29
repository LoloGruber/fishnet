#pragma once
#include <fishnet/GraphDecorator.hpp>

namespace fishnet::graph::__impl{

template<class G> requires (G::edge_type::isDirected()==true)
class DirectedAcyclicGraph: public GraphDecorator<DirectedAcyclicGraph<G>,G>{
private:
    using Base = GraphDecorator<DirectedAcyclicGraph<G>,G>;
    using N = typename G::node_type;
    using E = typename G::edge_type;

    /**
     * @brief Helper function to test if the insertion of an edge would form a cycle in the graph
     * @param from 
     * @param to 
     * @return true insertion of edge would create cycle
     * @return false graph would still be a DAG
     */
    bool hasCycleAfterAddingEdge(const N & from, const N & to){
        return false;
    }

public:
    DirectedAcyclicGraph(G && underlyingGraph):Base(std::move(underlyingGraph)){}

    bool addEdge(const N & from, const N & to){
        return true;
    }


};

}