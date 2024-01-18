#include "Edge.h"
#include <iostream>
#include "AdjacencyMap.h"
#include "AdjacencyContainer.h"
void test(){
    static_assert(graph::Edge<graph::UndirectedEdge<int>>);
    auto edge = graph::DirectedEdge<int>(2,1);
    auto weightFunction = [](int x, int y) {return x+y;};
    auto weighted = graph::Weighted<graph::UndirectedEdge<int>,int,decltype(weightFunction)>(2,5);
    auto otherWeighted = graph::Weighted<graph::UndirectedEdge<int>,int,decltype(weightFunction)>(3,4);
    auto dir = graph::DirectedEdge<int>(1,2);
    std::cout << (dir==edge) << std::endl;
    std::cout << (weighted == otherWeighted) << std::endl;
    std::cout << dir.hash() << " " << std::hash<graph::DirectedEdge<int>>{}(edge) << std::endl;
    std::cout << weighted.hash() << " " << std::hash<graph::Weighted<graph::UndirectedEdge<int>,int,decltype(weightFunction)>>{}(otherWeighted)<< std::endl;
}



void adjMap(){
    graph::AdjacencyMap<int> map;

    static_assert(graph::AdjacencyContainer<graph::AdjacencyMap<int>,int>);
    map.addAdjacency(1,2);
    map.addAdjacency(2,1);
    map.addNode(3);
    map.addAdjacency(1,3);
    auto adjList = map.adjacency(1);
    for(int i : adjList) {
        std::cout << i << std::endl;
    }
    for(int i: map.adjacency(4)) {
        std::cout << "nothing"<< i <<std::endl;
    }
    map.removeAdjacency(1,2);
    
    for(int i: map.adjacency(1)){
        std::cout << i << std::endl;
    }
    std::cout << "Nodes" <<std::endl;
    for(auto& n : map.nodes()) {
        std::cout << n << std::endl;
    }
    for(auto adj: map.getAdjacencyPairs()) {
        
        std::cout << adj.first << " -> " << adj.second << std::endl;
    }

    std::cout << std::endl;

    // for(const auto& adj: map.AdjacencyViews()) {
        
    //     std::cout << adj.first << " -> " << adj.second << std::endl;
    // }


}

template<graph::Edge E>
void printEdge(E const& edge){
    if constexpr(E::isDirected()){
        std::cout << edge.getFrom() << " -> " << edge.getTo() << std::endl;
    }else{
        std::cout << edge.getFrom() << " <-> " << edge.getTo() << std::endl;
    }

}


template<graph::WeightedEdge E>
void printWeightedEdge(E const& edge){
    if constexpr(E::isDirected()){
        std::cout << edge.getFrom() << " -> " << edge.getTo() << " ("<< edge.getWeight()<<")"<<std::endl;
    }else{
        std::cout << edge.getFrom() << " <-> " << edge.getTo() << " ("<< edge.getWeight()<<")"<<std::endl;
    }
}


#include "SimpleGraph.h"
#include "Graph.h"
#include "WeightedGraph.h"
void testGraph() {

    static_assert(graph::Graph<graph::UndirectedGraph<int>>);
    graph::UndirectedGraph<int> g;
    g.addEdge(1,2);
    g.addEdge(2,1);
    for(auto e: g.getEdges()){
        printEdge(e);
    }
    for(auto n: g.getReachableFrom(1)) {
        std::cout << n << std::endl;
    }

    graph::UndirectedGraph<int> x;
    std::vector<int> ints = {1,2,3};
    x.addNodes(ints);
    for (auto n : x.getNodes()){
        std::cout << n << std::endl;
    }
    auto weightFunction = [](int x, int y) {return x+y;};

    graph::WeightedDirectedGraph<int,int,decltype(weightFunction)> w;
    w.addEdge(1,2);
    w.addEdge(2,3);
    auto e = w.makeEdge(3,2);
    printWeightedEdge(e);
    for(auto e: w.getEdges()) {
        printWeightedEdge(e);
    }

}




int main(int argc, char const *argv[])
{
    test();
    adjMap();
    std::cout << std::endl;
    testGraph();
}
