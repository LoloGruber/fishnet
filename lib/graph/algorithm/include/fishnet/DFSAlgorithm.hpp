#pragma once
#include <stack>

#include <fishnet/CollectionConcepts.hpp>

#include <fishnet/Graph.hpp>

#include "SearchResult.hpp"
#include "SearchPath.hpp"
#include "DefaultBiPredicate.hpp"

namespace fishnet::graph::__impl {
/**
 * @brief Generic depth-first search implementation
 * 
 * @tparam G graph type
 * @tparam SearchResultImpl search result type
 * @param g graph
 * @param searchResult mutable reference to the search result
 * @param start start node
 * @param predicate BiPredicate to require additional criteria for two nodes to be in relation
 */
template<Graph G,class SearchResultImpl>
static void dfs(const G & g,SearchResult<SearchResultImpl> & searchResult, const typename G::node_type & start,  NodeBiPredicate<typename G::node_type>  auto const& predicate)  {
    using N = G::node_type;
    std::stack<N> nodes;
    nodes.push(start);
    searchResult.open(start);
    while(not nodes.empty() and not searchResult.stop()){
        N current = nodes.top();
        nodes.pop();
        if(searchResult.isOpen(current)) {
            for(auto & neighbour: g.getNeighbours(current)){
                if(searchResult.isUnknown(neighbour) and predicate(current,neighbour)) {
                    nodes.push(neighbour);
                    searchResult.open(neighbour);
                    searchResult.onEdge(current,neighbour);
                }
            }
            searchResult.close(current);
        }

    }
}
}

namespace fishnet::graph::DFS{
/**
 * @brief Find path between start and goal using depth-first search
 * 
 * @tparam G graph type
 * @param g graph
 * @param start start node
 * @param goal target node
 * @return util::input_range_of<typename G::edge_type> range of edges leading from start to goal
 */
template<Graph G>
static util::input_range_of<typename G::edge_type> auto findPath(const G & g,const typename G::node_type & start, const typename G::node_type & goal) {
    using N = typename G::node_type;
    using E = typename G::edge_type;
    using H = G::adj_container_type::hash_function;
    using Eq = G::adj_container_type::equality_predicate;
    auto searchPath = SearchPath<E,H,Eq>(goal);
    fishnet::graph::__impl::dfs(g,searchPath,start,__impl::DefaultBiPredicate<typename G::node_type>());
    auto pathOpt = searchPath.get();
    if (not pathOpt){
        return std::vector<E>();
    }
    std::vector<N> & nodes = *pathOpt;
    std::vector<E> edges;
    for (size_t i = 0; i < nodes.size()-1; i++){
        edges.emplace_back(g.makeEdge(nodes[i],nodes[i+1]));
    }
    return edges;
}
}


