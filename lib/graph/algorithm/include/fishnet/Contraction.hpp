#pragma once
#include "BFSAlgorithm.hpp"
#include <future>
#include <mutex>
#include <numeric>
#include <type_traits>

#include <fishnet/Graph.hpp>
#include <fishnet/NetworkConcepts.hpp>

#include <fishnet/CollectionConcepts.hpp>
#include <chrono>
namespace fishnet::graph::__impl {

template<typename N>
using QueueType = fishnet::util::BlockingQueue<std::pair<int,std::vector<N>>>;

template<typename N>
using QueuePtr = std::shared_ptr<QueueType<N>>;
/**
 * @brief Helper runnable, taking connected components and merging the vertices to a single vertex using the merge function and mapper for the first element
 * 
 * @tparam N source node type
 * @tparam R result node type
 * @param queue shared pointer to blocking queue, storing pairs of component-ids and vectors of nodes 
 * @param mergeFunction BinaryFunction on nodes, merges two result nodes into a single one of the result type
 * @param mapper UnaryFunction from source node type to result node type
 * @return std::vector<std::pair<int,N>> storing the pairs of component-id and the merged node
 */
template<typename N,typename R>
static std::vector<std::pair<int,R>> mergeWorker(QueuePtr<N> queue, util::BiOperator<R> auto const& mergeFunction, util::UnaryFunction<N,R> auto const & mapper){
    std::vector<std::pair<int,R>> mergeResult;
    auto current = fishnet::util::Element<std::pair<int,std::vector<N>>>::POISON_PILL;
    do{
        current = queue->take();
        if(not current) {
            queue->putPoisonPill();
            break;
        }
        const auto & [componentId, nodes] = current.get();
        R merged = mapper(nodes[0]);
        for(size_t i = 1; i < current.get().second.size(); i++){
            merged = mergeFunction(merged,mapper(nodes[i]));
            #if PERFORMANCE_TEST
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(std::chrono::microseconds(5390)); //todo estimate time for merging of polygons
            #endif
        }
        mergeResult.emplace_back(current.get().first,merged);
    }while(current != queue->getPoisonPill());
    return mergeResult;
}

/**
 * @brief Helper runnable for merging
 * Overload for equal source and result node types
 * @tparam N node type
 * @param queue shared pointer to blocking queue, storing pairs of component-ids and vectors of node
 * @param mergeFunction merging two nodes into a single node
 * @return std::vector<std::pair<int,N>> storing the pairs of component-id and the merged node
 */
template<typename N>
static std::vector<std::pair<int,N>> mergeWorker(QueuePtr<N> queue, util::BiOperator<N> auto const& mergeFunction){
    return mergeWorker<N,N>(queue,mergeFunction,util::Identity());
}

/**
 * @brief Helper runnable, taking connected components and merging the vertices to a single vertex using the contract function
 * 
 * @tparam N source node type
 * @tparam R result node type
 * @param queue shared pointer to blocking queue, storing pairs of component-ids and vectors of nodes 
 * @param reduceFunction function that reduces a range of nodes to a single node
 * @return std::vector<std::pair<int,R>> storing the pairs of component-id and the merged nodes (of type R)
 */
template<typename N,typename R>
static std::vector<std::pair<int,R>> mergeWorker(QueuePtr<N>  queue, util::ReduceFunction<std::vector<N>,R> auto const& reduceFunction){
    std::vector<std::pair<int,R>> mergeResult;
    auto current = fishnet::util::Element<std::pair<int,std::vector<N>>>::POISON_PILL;
    do{
        current = queue->take();
        if(not current) {
            queue->putPoisonPill();
            break;
        }
        const auto & [componentId, nodes] = current.get();
        mergeResult.emplace_back(componentId, reduceFunction(nodes));
    }while(current != queue->getPoisonPill());
    return mergeResult;
}

/**
 * @brief Helper runnable, taking connected components and merging the vertices to a single vertex using the contract function
 * 
 * @tparam N node type
 * @param queue shared pointer to blocking queue, storing pairs of component-ids and vectors of nodes 
 * @param reduceFunction function that reduces a range of nodes to a single node
 * @return std::vector<std::pair<int,N>> storing the pairs of component-id and the merged nodes
 */
template<typename N>
static std::vector<std::pair<int,N>> mergeWorker(QueuePtr<N> queue, util::ReduceFunction<std::vector<N>> auto const& reduceFunction){
    return mergeWorker(queue, reduceFunction,[](auto && node){return node;});
}

/**
 * @brief Helper Function to apply the merged components to the output graph 
 * 
 * @tparam SourceGraphType source graph type
 * @tparam TargetGraphType output graph type
 * @param source source graph (not changed)
 * @param output mutable reference to output graph
 * @param componentsMap map from source graph node type to component id (Source::node_type -> int)
 * @param result map from component id to output graph node type (int -> Target::node_type)
 */
template<Graph SourceGraphType, Graph TargetGraphType>
static inline void constructFromComponentsMap(const SourceGraphType & source, TargetGraphType & output, auto && componentsMap,  std::unordered_map<int,typename TargetGraphType::node_type> && result, auto const & mapper) noexcept {
    std::vector<typename TargetGraphType::node_type> disconnectedNodes;
    for(const auto & node: source.getNodes()){
        if(util::size(source.getNeighbours(node))==0 && util::size(source.getReachableFrom(node))==0){
            disconnectedNodes.emplace_back(mapper(node));
        }
    }
    output.addNodes(disconnectedNodes);
    output.addNodes(std::views::values(result)); // ensure that all nodes are added, even disconnected nodes after the merge
    std::vector<typename TargetGraphType::edge_type> edges;
    for(const auto & edge: source.getEdges()){
        if(componentsMap.at(edge.getFrom()) != componentsMap.at(edge.getTo())){
            edges.push_back(
                output.makeEdge(result.at(componentsMap.at(edge.getFrom())), result.at(componentsMap.at(edge.getTo())))
            );
        }
    }
    output.addEdges(edges);
}


/**
 * @brief Helper Function to apply the merged components in place to the output graph (required for Graph Databases)
 * 
 * @tparam SourceGraphType source graph type
 * @tparam TargetGraphType output graph type
 * @param source mutable reference to source graph
 * @param output mutable reference to output graph
 * @param componentsMap map from source graph node type to component id (Source::node_type -> int)
 * @param result map from component id to output graph node type (int -> Target::node_type)
 */
template<Graph SourceGraphType, Graph TargetGraphType>
static inline void constructFromComponentsMapInPlace(SourceGraphType & source, TargetGraphType & output, auto && componentsMap,  std::unordered_map<int,typename TargetGraphType::node_type> && result, auto const & mapper) noexcept {
    std::vector<typename TargetGraphType::node_type> disconnectedNodes;
    for(const auto & node: source.getNodes()){
        if(util::size(source.getNeighbours(node))==0 && util::size(source.getReachableFrom(node))==0){
            disconnectedNodes.emplace_back(mapper(node));
        }
    }
    std::vector<typename TargetGraphType::edge_type> edges;
    for(const auto & edge: source.getEdges()){
        if(componentsMap.at(edge.getFrom()) != componentsMap.at(edge.getTo())){
            edges.push_back(
                output.makeEdge(result.at(componentsMap.at(edge.getFrom())), result.at(componentsMap.at(edge.getTo())))
            );
        }
    }
    source.clear();
    output.addNodes(disconnectedNodes);
    output.addNodes(std::views::values(result)); // ensure that all nodes are added, even disconnected nodes after the merge
    output.addEdges(edges);
}

}

namespace fishnet::graph{

    /**
 * @brief Generic contract implementation. Edges between nodes fulfilling the contractBiPredicate get contracted. 
 * The incident nodes get merged by first mapping to the node type of the result graph and combining them using the mergeFunction
 * 
 * @tparam SourceGraphType source graph type
 * @tparam TargetGraphType target graph type
 * @param source source graph (not changed)
 * @param contractBiPredicate specifies when an edge between two nodes from source graph shall be contracted
 * @param mergeFunction specifies how nodes of the target graph will be merged
 * @param mapper maps nodes from the source graph to the node type of the target graph
 * @param output mutable reference to the target graph
 * @param workers amount of concurrent works
 * @return TargetGraphType resulting graph after contracting edges and merging nodes using the target graph node type
 */
template<Graph SourceGraphType, Graph TargetGraphType>
void contract(const SourceGraphType & source, util::BiPredicate<typename SourceGraphType::node_type> auto const & contractBiPredicate,util::BiFunction<typename SourceGraphType::node_type,typename SourceGraphType::node_type,typename TargetGraphType::node_type> auto const & mergeFunction,
 util::UnaryFunction<typename SourceGraphType::node_type,typename TargetGraphType::node_type> auto const & mapper, TargetGraphType & output,  u_int8_t workers = 1)
{
    using N = SourceGraphType::node_type;
    using R = TargetGraphType::node_type;
    if(workers == 0) 
        workers = 1;
    auto queue = std::make_shared<__impl::QueueType<N>>();
    std::vector<std::future<std::vector<std::pair<int,R>>>> futures;
    futures.reserve(workers);
    for(int i = 0; i < workers; i++){
        futures.emplace_back(std::async(std::launch::async,[queue,&mergeFunction,&mapper](){return __impl::mergeWorker<N,R>(queue,mergeFunction,mapper);}));
    }
    auto componentsMap = BFS::connectedComponents(source,queue,contractBiPredicate).asMap();
    std::unordered_map<int,R> result;
    queue->putPoisonPill();
    result.reserve(componentsMap.size());
    for(auto & f: futures){
        for(auto & merged: f.get() ){
            result.try_emplace(merged.first,merged.second);
        }
    }
    __impl::constructFromComponentsMap(source,output,componentsMap,std::move(result),mapper);
}

/**
 * @brief Generic contract implementation. Edges between nodes fulfilling the contractBiPredicate get contracted. 
 * The incident nodes get merge by reducing a vector of nodes from the source graph to a single node of the target graph
 * 
 * @tparam SourceGraphType source graph type
 * @tparam TargetGraphType target graph type
 * @param source source graph (not changed)
 * @param contractBiPredicate specifies when an edge between two nodes from source graph shall be contracted
 * @param reduceFunction specifies how a vector of nodes from the source graph get combined to a single node of the target graph
 * @param output mutable reference to the target graph
 * @param workers amount of concurrent works
 * @return TargetGraphType resulting graph after contracting edges and merging nodes using the target graph node type
 */
template<Graph SourceGraphType, Graph TargetGraphType>
void contract( const SourceGraphType & source, util::BiPredicate<typename SourceGraphType::node_type> auto const & contractBiPredicate,
    util::ReduceFunction<std::vector<typename SourceGraphType::node_type>,typename TargetGraphType::node_type> auto const & reduceFunction, TargetGraphType & output,  u_int8_t workers = 1)
{
    using N = SourceGraphType::node_type;
    using R = TargetGraphType::node_type;
    if(workers == 0) 
        workers = 1;
    auto queue = std::make_shared<__impl::QueueType<N>>();
    std::vector<std::future<std::vector<std::pair<int,R>>>> futures;
    futures.reserve(workers);
    for(int i = 0; i < workers; i++){
        futures.emplace_back(std::async(std::launch::async,[queue,&reduceFunction](){return __impl::mergeWorker<N,R>(queue,reduceFunction);}));
    }
    auto componentsMap = BFS::connectedComponents(source,queue,contractBiPredicate).asMap();
    std::unordered_map<int,R> result;
    queue->putPoisonPill();
    result.reserve(componentsMap.size());
    for(auto & f: futures){
        for(auto & merged: f.get() ){
            result.try_emplace(merged.first,merged.second);
        }
    }
    auto mapper = [&reduceFunction](const N & node){return reduceFunction(std::vector<N>({node}));};
    __impl::constructFromComponentsMap(source,output,std::move(componentsMap),std::move(result),mapper);
}

/**
 * @brief Generic in place contraction implementation. Edges between nodes fulfilling the contractBiPredicate get contracted. 
 * The incident nodes get merge by reducing a vector of nodes from the source graph to a single node of the target graph
 * 
 * @tparam SourceGraphType source graph type
 * @tparam TargetGraphType target graph type
 * @param source mutable reference to source graph, gets cleared during the contraction
 * @param contractBiPredicate specifies when an edge between two nodes from source graph shall be contracted
 * @param reduceFunction specifies how a vector of nodes from the source graph get combined to a single node of the target graph
 * @param output mutable reference to the target graph
 * @param workers amount of concurrent works
 * @return TargetGraphType resulting graph after contracting edges and merging nodes using the target graph node type
 */
template<Graph SourceGraphType, Graph TargetGraphType>
void contractInPlace( SourceGraphType & source, util::BiPredicate<typename SourceGraphType::node_type> auto const & contractBiPredicate,
    util::ReduceFunction<std::vector<typename SourceGraphType::node_type>,typename TargetGraphType::node_type> auto const & reduceFunction, TargetGraphType & output,  u_int8_t workers = 1)
{
    using N = SourceGraphType::node_type;
    using R = TargetGraphType::node_type;
    if(workers == 0) 
        workers = 1;
    auto queue = std::make_shared<__impl::QueueType<N>>();
    std::vector<std::future<std::vector<std::pair<int,R>>>> futures;
    futures.reserve(workers);
    for(int i = 0; i < workers; i++){
        futures.emplace_back(std::async(std::launch::async,[queue,&reduceFunction](){return __impl::mergeWorker<N,R>(queue,reduceFunction);}));
    }
    auto componentsMap = BFS::connectedComponents(source,queue,contractBiPredicate).asMap();
    std::unordered_map<int,R> result;
    queue->putPoisonPill();
    result.reserve(componentsMap.size());
    for(auto & f: futures){
        for(auto & merged: f.get() ){
            result.try_emplace(merged.first,merged.second);
        }
    }
    auto mapper = [&reduceFunction](const N & node){return reduceFunction(std::vector<N>({node}));};
    __impl::constructFromComponentsMapInPlace(source,output,std::move(componentsMap),std::move(result),mapper);
}

/**
 * @brief Generic in place contraction implementation. Edges between nodes fulfilling the contractBiPredicate get contracted. 
 * The incident nodes get merge by reducing a vector of nodes from the source graph to a single node of the target graph
 * 
 * @tparam SourceGraphType source graph type
 * @tparam TargetGraphType target graph type
 * @param graph mutable reference to graph to be contracted
 * @param contractBiPredicate specifies when an edge between two nodes from source graph shall be contracted
 * @param reduceFunction specifies how a vector of nodes from the source graph get combined to a single node of the target graph
 * @param workers amount of concurrent works
 * @return TargetGraphType resulting graph after contracting edges and merging nodes using the target graph node type
 */
template<Graph SourceGraphType>
void contractInPlace( SourceGraphType & graph, util::BiPredicate<typename SourceGraphType::node_type> auto const & contractBiPredicate,
    util::ReduceFunction<std::vector<typename SourceGraphType::node_type>,typename SourceGraphType::node_type> auto const & reduceFunction, u_int8_t workers = 1)
{
    using N = SourceGraphType::node_type;
    using R = SourceGraphType::node_type;
    if(workers == 0) 
        workers = 1;
    auto queue = std::make_shared<__impl::QueueType<N>>();
    std::vector<std::future<std::vector<std::pair<int,R>>>> futures;
    futures.reserve(workers);
    for(int i = 0; i < workers; i++){
        futures.emplace_back(std::async(std::launch::async,[queue,&reduceFunction](){return __impl::mergeWorker<N,R>(queue,reduceFunction);}));
    }
    auto componentsMap = BFS::connectedComponents(graph,queue,contractBiPredicate).asMap();
    std::unordered_map<int,R> result;
    queue->putPoisonPill();
    result.reserve(componentsMap.size());
    for(auto & f: futures){
        for(auto & merged: f.get() ){
            result.try_emplace(merged.first,merged.second);
        }
    }
    auto mapper = [&reduceFunction](const N & node){return reduceFunction(std::vector<N>({node}));};
    __impl::constructFromComponentsMapInPlace(graph,graph,std::move(componentsMap),std::move(result),mapper);
}

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
template<Graph G> requires std::is_constructible_v<G>
G contract(const G & source, NodeBiPredicate<typename G::node_type> auto const& contractBiPredicate, NodeBiOperator<typename G::node_type> auto const& mergeFunction, u_int8_t workers = 1){
    G result;
    contract(source,contractBiPredicate,mergeFunction,util::Identity(),result,workers);
    return result;
}

/**
 * @brief Contract the graph depending on the contraction predicate, applying the reduce function to merge clustered nodes
 * 
 * @tparam G graph type
 * @param source graph
 * @param contractBiPredicate BiPredicate, deciding whether an edge between two nodes should be contracted
 * @param reduceFunction Reduce function, defining how a range of nodes should be reduced to a single node
 * @param workers amount of worker threads
 * @return G contracted graph
 */
template<Graph G> requires std::is_constructible_v<G>
G contract(const G & source, NodeBiPredicate<typename G::node_type> auto const& contractBiPredicate, util::ReduceFunction<std::vector<typename G::node_type>> auto const& reduceFunction, u_int8_t workers = 1){
    G result;
    contract(source,contractBiPredicate,reduceFunction,result,workers);
    return result;
}   
}
