#pragma once
#include <concepts>
#include <vector>
#include <array>
#include <queue>
#include <memory>
#include <type_traits>
#include <algorithm>
#include <fishnet/BlockingQueue.hpp>
#include <fishnet/Graph.hpp>
#include <fishnet/NetworkConcepts.hpp>

#include "ConnectedComponents.hpp"
#include "ConcurrentConnectedComponents.hpp"
#include "SearchResult.hpp"
#include "SearchPath.hpp"
#include "DefaultBiPredicate.hpp"

namespace fishnet::graph::__impl{

/**
 * @brief Generic breadth-first search implementation starting from a node
 * 
 * @tparam G graph type
 * @tparam SearchResultImpl search result type (using CRTP)
 * @param g graph
 * @param searchResult mutable reference to search result
 * @param start start node
 * @param predicate BiPredicate to require additional criteria for two nodes to be in relation
 */
template<Graph G, class SearchResultImpl>
static void bfs(const G  & g, SearchResult<SearchResultImpl> & searchResult, const  typename G::node_type  & start,NodeBiPredicate<typename G::node_type> auto const& predicate) {
    using N = G::node_type;
    std::queue<N> q;
    q.push(start);
    while(not q.empty()and not searchResult.stop()) {
        N current = q.front();
        q.pop();
        searchResult.open(current);
        for(auto & neighbour : g.getNeighbours(current)){
                if (searchResult.isUnknown(neighbour) and predicate(current,neighbour) ) {
                    searchResult.open(neighbour);
                    searchResult.onEdge(current,neighbour);
                    q.push(neighbour);
                }
        }
        searchResult.close(current);
    }
}

/**
 * @brief Generic breadth-first search implementation, traversing the entire graph
 * 
 * @tparam G graph type
 * @tparam SearchResultImpl search result type (using CRTP)
 * @param g graph
 * @param searchResult mutable reference to search result
 * @param predicate BiPredicate to require additional criteria for two nodes to be in relation
 */
template<Graph G,class SearchResultImpl>
static void bfs_all(const Graph auto & g, SearchResult<SearchResultImpl> & searchResult,NodeBiPredicate<typename G::node_type> auto const& predicate){
    auto nodes = g.getNodes();
    for(auto const& n : nodes){
        if (searchResult.stop()){
            return;
        }
        if(searchResult.isUnknown(n)){
            bfs(g, searchResult,n,predicate);
        }
    }
}
}

namespace fishnet::graph::BFS {

/**
 * @brief Compute connected components of the graph
 * 
 * @tparam G graph type
 * @param graph graph
 * @param inRelation BiPredicate indicating whether two nodes are in relation
 * @return ConnectedComponents search result
 */
template<Graph G>
auto connectedComponents(const  G  & graph, NodeBiPredicate<typename G::node_type> auto const& inRelation)  {
    using H = G::adj_container_type::hash_function;
    using E = G::adj_container_type::equality_predicate;
    auto connectedComponents = ConnectedComponents<typename G::node_type, H, E>();
    __impl::bfs_all<G>(graph,connectedComponents,inRelation);
    return connectedComponents;
}

/**
 * @brief Compute connected components of the graph\
 * Uses  DefaultBiPredicate which is always true, when two nodes share an edge
 * @tparam G graph type
 * @param graph graph
 * @return ConnectedComponents search result
 */
template<Graph G>
auto connectedComponents(const G  & graph) {
    return connectedComponents(graph,__impl::DefaultBiPredicate<typename G::node_type>());
}

/**
 * @brief Compute connected components of the graph concurrently
 * 
 * @tparam G graph type
 * @param graph graph
 * @param queue shared pointer to blocking queue, storing pairs of component-ids and vectors of nodes
 * @param inRelation BiPredicate indicating whether two nodes are in relation
 * @return ConcurrentConnectedComponents search result 
 */
template<Graph G>
auto connectedComponents(const G & graph,std::shared_ptr<fishnet::util::BlockingQueue<std::pair<int,std::vector<typename G::node_type>>>>  queue, NodeBiPredicate<typename G::node_type> auto const& inRelation)  {
    using H = G::adj_container_type::hash_function;
    using E = G::adj_container_type::equality_predicate;
    auto concurrentConnectedComponents = ConcurrentConnectedComponents<typename G::node_type,H,E>(queue);
    __impl::bfs_all<G>(graph,concurrentConnectedComponents,inRelation);
    return concurrentConnectedComponents;
}

/**
 * @brief Compute connected components of the graph concurrently
 * 
 * @tparam G graph type
 * @param graph graph
 * @param queue shared pointer to blocking queue, storing pairs of component-ids and vectors of nodes
 * @return ConcurrentConnectedComponents search result  
 */
template<Graph G>
auto connectedComponents(const G & graph, std::shared_ptr<fishnet::util::BlockingQueue<std::pair<int,std::vector<typename G::node_type>>>>  queue)  {
    return connectedComponents(graph,queue,__impl::DefaultBiPredicate<typename G::node_type>());
}

/**
 * @brief Find breadth-first search between between start and goal node
 * 
 * @tparam G graph type
 * @param graph graph
 * @param start start node
 * @param goal target node
 * @return util::input_range_of<typename G::edge_type>  vector of edges from start to target
 */
template<Graph G>
util::input_range_of<typename G::edge_type> auto findPath(const G & graph, const typename G::node_type & start, const typename G::node_type & goal) {
    using H = G::adj_container_type::hash_function;
    using E = G::adj_container_type::equality_predicate;
    using N = G::node_type;
    auto p = SearchPath<typename G::edge_type,H,E>(goal);
     __impl::bfs(graph,p,start,__impl::DefaultBiPredicate<N>());
    std::optional<std::vector<N>> nodesOpt = p.get();
    if (not nodesOpt){
        return std::vector<typename G::edge_type>();
    }
    std::vector<N> nodes = *nodesOpt;
    std::vector<typename G::edge_type> edges;
    for(size_t i = 0; i < nodes.size()-1; i++) {
        edges.push_back(graph.makeEdge(nodes[i],nodes[i+1]));
    }
    return edges;
}
}