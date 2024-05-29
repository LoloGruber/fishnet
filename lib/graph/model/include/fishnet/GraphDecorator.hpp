#pragma once
#include <fishnet/GraphModel.hpp>
#include <fishnet/AbstractGraph.hpp>
#include <fishnet/CollectionConcepts.hpp>
namespace fishnet::graph{
/**
 * @brief Abstract Graph Decorator using CRTP
 * 
 * @tparam DecoratorImpl graph decorator implementation type
 * @tparam G graph type
 * @tparam G::edge_type edge type of graph
 */
template<class DecoratorImpl, class G, Edge E = typename G::edge_type>
class GraphDecorator : public AbstractGraph<DecoratorImpl, E, typename G::adj_container_type>
{                                                          
protected:
    using Base = AbstractGraph<DecoratorImpl,E,typename G::adj_container_type>;
    G g;
    using N = G::node_type;
    GraphDecorator():Base(){}
    GraphDecorator(G && source):g(std::move(source)){};
    GraphDecorator(typename G::adj_container_type && adjContainer):g(std::move(adjContainer)){}
    GraphDecorator(util::forward_range_of<N> auto& nodes):g(nodes){};
    GraphDecorator(util::forward_range_of<N> auto && nodes):g(nodes){};
public:
    using edge_type = E;
    using adj_container_type = G::adj_container_type;
    using node_type = N;

    bool addNode(N & node){
        return g.addNode(node);
    }

    bool addNode( N && node){
        return g.addNode(node);
    }

    template<typename... Args>
    bool addNode(const N & node, Args... args){
        return g.addNode(node,args...);
    }

    template<typename... Args>
    bool addNode(N && node, Args... args){
        return g.addNode(std::move(node),args...);
    }

    bool addNodes(util::forward_range_of<N> auto & nodes){
        return g.addNodes(nodes);
    }

    bool addNodes(util::forward_range_of<N> auto && nodes){
        return g.addNodes(nodes);
    }

    bool containsNode(const N & node) const noexcept {
        return g.containsNode(node);
    }

    void removeNode(const N & node) {
        g.removeNode(node);
    }

    bool addEdge(const N & from, const N & to){
        return g.addEdge(from,to);
    }

    bool addEdge(N && from, N && to){
        return g.addEdge(std::move(from),std::move(to));
    }

    bool addEdge(const E & edge){
        return g.addEdge(edge);
    }

    void addEdges(util::forward_range_of<std::pair<N,N>> auto && edges){
        g.addEdges(edges);
    }

    void addEdges(util::forward_range_of<E> auto  && edges) {
        g.addEdges(edges);
    }

    bool containsEdge(const N & from, const N & to) const noexcept{
        return g.containsEdge(from,to);
    }

    bool containsEdge(const E & edge) const noexcept {
        return g.containsEdge(edge);
    }

    void removeEdge(const N & from, const N & to){
        g.removeEdge(from,to);
    }

    void removeEdge(const E & edge){
        g.removeEdge(edge);
    }

    inline E makeEdge(const N & from, const N & to) const {
        return g.makeEdge(from,to);
    }

    auto getNodes() const noexcept{
        return g.getNodes();
    }

    auto getEdges() const {
        return g.getEdges();
    }

    auto getNeighbours(const N & node) const noexcept {
        return g.getNeighbours(node);
    }

    auto getReachableFrom(const N & node) const noexcept {
        return g.getReachableFrom(node);
    }

    auto getOutboundEdges(const N & node) const {
        return g.getOutboundEdges(node);
    }

    auto getInboundEdges(const N & node) const {
        return g.getInboundEdges(node);
    }

    void clear(){
        g.clear();
    }

    virtual ~GraphDecorator() = default;   
};
}
