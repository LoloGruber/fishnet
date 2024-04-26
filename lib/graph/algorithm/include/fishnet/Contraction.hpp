#pragma once
#include "BFSAlgorithm.hpp"
#include <future>
#include <mutex>
#include <numeric>

#include <fishnet/Graph.hpp>
#include <fishnet/NetworkConcepts.hpp>

#include <fishnet/CollectionConcepts.hpp>
#include <chrono>
namespace fishnet::graph{

namespace __impl {
template<typename N>
static std::vector<std::pair<int,N>> mergeWorker(std::shared_ptr<BlockingQueue<std::pair<int,std::vector<N>>>>  queue, NodeBiOperator<N> auto const& contractFunction){
    std::vector<std::pair<int,N>> mergeResult;
    Element<std::pair<int,std::vector<N>>> current = Element<std::pair<int,std::vector<N>>>::POISON_PILL;
    using namespace std::chrono_literals;
    do{
        current = queue->take();
        if(not current) {
            queue->putPoisonPill();
            break;
        }
        N merged = current.get().second[0];
        for(size_t i = 1; i < current.get().second.size(); i++){
            merged = contractFunction(merged,current.get().second[i]);
            #if PERFORMANCE_TEST
            std::this_thread::sleep_for(std::chrono::microseconds(5390)); //todo estimate time for merging of polygons
            #endif
        }
        mergeResult.push_back(std::pair<int,N>(current.get().first,merged));
    }while(current != queue->getPoisonPill());
    return mergeResult;
}

template<typename N>
static std::vector<std::pair<int,N>> mergeWorker(std::shared_ptr<BlockingQueue<std::pair<int,std::vector<N>>>>  queue, util::ReduceFunction<std::vector<N>> auto const& reduceFunction){
    std::vector<std::pair<int,N>> mergeResult;
    Element<std::pair<int,std::vector<N>>> current = Element<std::pair<int,std::vector<N>>>::POISON_PILL;
    using namespace std::chrono_literals;
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

template<Graph G>
G contract(const G & source, NodeBiPredicate<typename G::node_type> auto const& mergePredicate, NodeBiOperator<typename G::node_type> auto const& contractFunction, u_int8_t workers = 1){
    using N = G::node_type;
    if(workers == 0) workers =1;
    auto queue = std::make_shared<BlockingQueue<std::pair<int,std::vector<N>>>>();
    std::stop_source interrupt;
    std::vector<std::future<std::vector<std::pair<int,N>>>> futures;
    futures.reserve(workers);
    for(int i = 0; i < workers; i++){
        futures.emplace_back(std::async(std::launch::async,[queue,contractFunction](){return __impl::mergeWorker(queue,contractFunction);}));
    }
    //auto componentsFuture = std::async(std::launch::async,[source,queue](){ return BFSAlgorithm<GraphImpl>(source).template connectedComponents<Predicate>(queue);}); // just c++ things... computes connected components concurrently
    auto componentsMap = BFS::connectedComponents(source,queue,mergePredicate).asMap();
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

template<Graph G>
G contract(const G & source, NodeBiPredicate<typename G::node_type> auto const& mergePredicate, util::ReduceFunction<std::vector<typename G::node_type>> auto const& reduceFunction, u_int8_t workers = 1){
    using N = G::node_type;
    if(workers == 0) workers =1;
    auto queue = std::make_shared<BlockingQueue<std::pair<int,std::vector<N>>>>();
    std::stop_source interrupt;
    std::vector<std::future<std::vector<std::pair<int,N>>>> futures;
    futures.reserve(workers);
    for(int i = 0; i < workers; i++){
        futures.emplace_back(std::async(std::launch::async,[queue,reduceFunction](){return __impl::mergeWorker(queue,reduceFunction);}));
    }
    //auto componentsFuture = std::async(std::launch::async,[source,queue](){ return BFSAlgorithm<GraphImpl>(source).template connectedComponents<Predicate>(queue);}); // just c++ things... computes connected components concurrently
    auto componentsMap = BFS::connectedComponents(source,queue,mergePredicate).asMap();
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
