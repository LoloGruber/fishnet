#pragma once
#include "Graph.h"
#include <vector>
namespace fishnet::graph {
struct DegreeCentrality{
    template<Graph G>
    std::vector<std::pair<typename G::node_type,size_t>> operator() (const G & graph) const{
        std::vector<std::pair<typename G::node_type,size_t>> result;
        auto nodes = graph.getNodes();
        result.reserve(nodes.size()); //TODO adapt for ranges that do not suppert .size()
        std::transform(nodes.begin(),nodes.end(),std::back_inserter(result),[&graph]( const auto &  node){return std::make_pair(node,graph.getNeighbours(node).size());}); 
        return result;
    }
};
}