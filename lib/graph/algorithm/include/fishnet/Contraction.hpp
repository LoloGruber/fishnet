#pragma once
#include "BFSAlgorithm.hpp"
#include <future>
#include <mutex>
#include <numeric>

#include <fishnet/Graph.hpp>
#include <fishnet/NetworkConcepts.hpp>

#include <fishnet/CollectionConcepts.hpp>
#include <chrono>
namespace fishnet::graph::__impl {
/**
 * @brief Helper runnable, taking connected components and merging the vertices to a single vertex using the merge function
 * 
 * @tparam N node type
 * @param queue shared pointer to blocking queue, storing pairs of component-ids and vectors of nodes 
 * @param mergeFunction BiOperator on nodes, merges two nodes into a single one
 * @return std::vector<std::pair<int,N>> storing the pairs of component-id and the merged node
 */
template<typename N>
static std::vector<std::pair<int,N>> mergeWorker(std::shared_ptr<fishnet::util::BlockingQueue<std::pair<int,std::vector<N>>>>  queue, NodeBiOperator<N> auto const& mergeFunction){
    std::vector<std::pair<int,N>> mergeResult;
    auto current = fishnet::util::Element<std::pair<int,std::vector<N>>>::POISON_PILL;
    do{
        current = queue->take();
        if(not current) {
            queue->putPoisonPill();
            break;
        }
        N merged = current.get().second[0];
        for(size_t i = 1; i < current.get().second.size(); i++){
            merged = mergeFunction(merged,current.get().second[i]);
            #if PERFORMANCE_TEST
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(std::chrono::microseconds(5390)); //todo estimate time for merging of polygons
            #endif
        }
        mergeResult.push_back(std::pair<int,N>(current.get().first,merged));
    }while(current != queue->getPoisonPill());
    return mergeResult;
}

/**
 * @brief Helper runnable, taking connected components and merging the vertices to a single vertex using the contract function
 * 
 * @tparam N node type
 * @param queue shared pointer to blocking queue, storing pairs of component-ids and vectors of nodes 
 * @param reduceFunction function that reduces a range of nodes to a single node
 * @return std::vector<std::pair<int,N>> storing the pairs of component-id and the merged nod
 */
template<typename N>
static std::vector<std::pair<int,N>> mergeWorker(std::shared_ptr<fishnet::util::BlockingQueue<std::pair<int,std::vector<N>>>>  queue, util::ReduceFunction<std::vector<N>> auto const& reduceFunction){
    std::vector<std::pair<int,N>> mergeResult;
    auto current = fishnet::util::Element<std::pair<int,std::vector<N>>>::POISON_PILL;
    do{
        current = queue->take();
        if(not current) {
            queue->putPoisonPill();
            break;
        }
        N merged = reduceFunction(current.get().second);
        mergeResult.push_back(std::pair<int,N>(current.get().first,merged));
    }while(current != queue->getPoisonPill());
    return mergeResult;
}
}

namespace fishnet::graph{
/**
 * @brief Contract the graph depending on the contraction predicate, applying the merge function on the nodes of contracted edges
 * 
 * @tparam G graph type
 * @param source graph
 * @param contractBiPredicate BiPredicate, deciding whether an edge between two nodes should be contracted
 * @param mergeFunction BiOperator, defining how two nodes should be merge into a single node
 * @param workers amount of worker threads
 * @return G contracted graph
 */
template<Graph G>
G contract(const G & source, NodeBiPredicate<typename G::node_type> auto const& contractBiPredicate, NodeBiOperator<typename G::node_type> auto const& mergeFunction, u_int8_t workers = 1){
    using N = G::node_type;
    if(workers == 0) workers =1;
    auto queue = std::make_shared<fishnet::util::BlockingQueue<std::pair<int,std::vector<N>>>>();
    std::stop_source interrupt;
    std::vector<std::future<std::vector<std::pair<int,N>>>> futures;
    futures.reserve(workers);
    for(int i = 0; i < workers; i++){
        futures.emplace_back(std::async(std::launch::async,[queue,mergeFunction](){return __impl::mergeWorker(queue,mergeFunction);}));
    }
    //auto componentsFuture = std::async(std::launch::async,[source,queue](){ return BFSAlgorithm<GraphImpl>(source).template connectedComponents<Predicate>(queue);}); // just c++ things... computes connected components concurrently
    auto componentsMap = BFS::connectedComponents(source,queue,contractBiPredicate).asMap();
    std::unordered_map<int,N> result;
    queue->putPoisonPill();
    result.reserve(componentsMap.size());
    for(auto & f: futures){
        for(auto & merged: f.get() ){
            result.try_emplace(merged.first,merged.second);
        }
    }
    G output;
    for(auto & e: source.getEdges()){
        if(componentsMap.at(e.getFrom())!=componentsMap.at(e.getTo())) {
            int indexFrom = componentsMap.at(e.getFrom());
            N from = result.at(indexFrom);
            N to = result.at(componentsMap.at(e.getTo()));
            output.addEdge(from,to);   
        }
    }
    return output;
}

/**
 * @brief Contract the graph depending on the contraction predicate, applying the merge function on the nodes of contracted edges
 * 
 * @tparam G graph type
 * @param source graph
 * @param contractBiPredicate BiPredicate, deciding whether an edge between two nodes should be contracted
 * @param reduceFunction Reduce function, defining how a range of nodes should be reduced to a single node
 * @param workers amount of worker threads
 * @return G contracted graph
 */
template<Graph G>
G contract(const G & source, NodeBiPredicate<typename G::node_type> auto const& contractBiPredicate, util::ReduceFunction<std::vector<typename G::node_type>> auto const& reduceFunction, u_int8_t workers = 1){
    using N = G::node_type;
    if(workers == 0) workers =1;
    auto queue = std::make_shared<fishnet::util::BlockingQueue<std::pair<int,std::vector<N>>>>();
    std::stop_source interrupt;
    std::vector<std::future<std::vector<std::pair<int,N>>>> futures;
    futures.reserve(workers);
    for(int i = 0; i < workers; i++){
        futures.emplace_back(std::async(std::launch::async,[queue,reduceFunction](){return __impl::mergeWorker(queue,reduceFunction);}));
    }
    //auto componentsFuture = std::async(std::launch::async,[source,queue](){ return BFSAlgorithm<GraphImpl>(source).template connectedComponents<Predicate>(queue);}); // just c++ things... computes connected components concurrently
    auto componentsMap = BFS::connectedComponents(source,queue,contractBiPredicate).asMap();
    std::unordered_map<int,N> result;
    queue->putPoisonPill();
    result.reserve(componentsMap.size());
    for(auto & f: futures){
        for(auto & merged: f.get() ){
            result.try_emplace(merged.first,merged.second);
        }
    }
    G output;
    for(auto  n: source.getNodes()){
        if(util::size(source.getNeighbours(n)) == 0 && util::size(source.getReachableFrom(n)) == 0){
            output.addNode(n); // add disconnected nodes
        }
    }
    for(auto & e: source.getEdges()){
        if(componentsMap.at(e.getFrom())!=componentsMap.at(e.getTo())) {
            int indexFrom = componentsMap.at(e.getFrom());
            N from = result.at(indexFrom);
            N to = result.at(componentsMap.at(e.getTo()));
            output.addEdge(from,to);   
        }
    }
    return output;
}   
}
