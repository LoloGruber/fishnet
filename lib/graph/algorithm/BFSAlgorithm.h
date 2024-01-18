#ifndef Breadth_First_Search_Algorithms_H
#define Breadth_First_Search_Algorithms_H
#include <concepts>
#include <vector>
#include <array>
#include <queue>
#include <memory>
#include <type_traits>
#include <algorithm>

#include "Graph.h"
#include "NetworkConcepts.h"
#include "ConnectedComponents.h"
#include "ConcurrentConnectedComponents.h"
#include "SearchResult.h"
#include "SearchPath.h"
#include "BlockingQueue.h"
#include "DefaultBiPredicate.h"


namespace fishnet::graph{


namespace __impl{
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





template<Graph G,class SearchResultImpl, NodeBiPredicate<typename G::node_type> Predicate = DefaultBiPredicate<typename G::node_type>>
static void bfs_all(const Graph auto & g, SearchResult<SearchResultImpl> & searchResult,NodeBiPredicate<typename G::node_type> auto const& predicate){
    auto nodes = g.getNodes();
    for(auto n : nodes){
        if (searchResult.stop()){
            return;
        }
        if(searchResult.isUnknown(n)){
            bfs(g, searchResult,n,predicate);
        }
    }
}
}

namespace BFS {

template<Graph G>
auto connectedComponents(const  G  & graph, NodeBiPredicate<typename G::node_type> auto const& inRelation)  {
    using H = G::adj_container_type::hash_function;
    using E = G::adj_container_type::equality_predicate;
    auto connectedComponents = ConnectedComponents<typename G::node_type, H, E>();
    __impl::bfs_all<G>(graph,connectedComponents,inRelation);
    return connectedComponents;
}

template<Graph G>
auto connectedComponents(const G  & graph) {
    return connectedComponents(graph,__impl::DefaultBiPredicate<typename G::node_type>());
}

template<Graph G>
auto connectedComponents(const G & graph,std::shared_ptr<BlockingQueue<std::pair<int,std::vector<typename G::node_type>>>>  q, NodeBiPredicate<typename G::node_type> auto const& inRelation)  {
    using H = G::adj_container_type::hash_function;
    using E = G::adj_container_type::equality_predicate;
    auto concurrentConnectedComponents = ConcurrentConnectedComponents<typename G::node_type,H,E>(q);
    __impl::bfs_all<G>(graph,concurrentConnectedComponents,inRelation);
    return concurrentConnectedComponents;
}
template<Graph G>
auto connectedComponents(const G & graph, std::shared_ptr<BlockingQueue<std::pair<int,std::vector<typename G::node_type>>>>  q)  {
    return connectedComponents(graph,q,__impl::DefaultBiPredicate<typename G::node_type>());
}

template<Graph G>
auto findPath(const G & graph, const typename G::node_type & start, const typename G::node_type & goal) {
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
}


#endif