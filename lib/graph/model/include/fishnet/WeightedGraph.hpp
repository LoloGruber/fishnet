#pragma once
#include "Graph.hpp"
#include "Edge.hpp"
#include "SimpleGraph.hpp"
#include "NetworkConcepts.hpp"
#include "GraphDecorator.hpp"
#include <unordered_map>

namespace fishnet::graph{
/**
 * @brief Interface for a weighted graph
 * 
 * @tparam GraphImpl weighted graph implementation type
 * @tparam E edge type required to be a Weighted Edge
 * @tparam N node type
 */
template<typename GraphImpl, typename E= GraphImpl::edge_type, typename N= E::node_type>
concept WeightedGraph = Node<N> && WeightedEdge<E> && Graph<GraphImpl,E,N>;
}

namespace fishnet::graph::__impl {
    /**
     * @brief Weighted graph decorator
     * 
     * @tparam G graph type
     * @tparam A annotation type
     * @tparam W weight function type
     */
    template<class G,Annotation A, WeightFunction<typename G::node_type,A> W>
    class WeightedGraphDecorator: public GraphDecorator<WeightedGraphDecorator<G,A,W>,G,WeightEdge<typename G::edge_type,A,W>>{
        private:
            using E = G::edge_type;
            using WE= WeightEdge<E,A,W>;
            using N = G::node_type;
            using WeightMap = std::unordered_map<E,A>;
            using Base = GraphDecorator<WeightedGraphDecorator<G,A,W>,G,WE>;
            WeightMap weightMap; // weights are stored in a map from edge to annotation, allowing heterogenous weights retrieved by the weight function or by the user
        public:
            using edge_type=WE;
            using adj_container_type = G::adj_container_type;
            static_assert(! WeightedEdge<E, typename E::node_type, A,W>); // underlying edge type must not be weighted
        private:
            inline WE toWeightedEdge(const E & e)const{
                if (weightMap.contains(e)){
                    return WE(e.getFrom(),e.getTo(),weightMap.at(e));
                }
                return WE(e);
            }

            std::vector<WE> toWeightedEdges(util::forward_range_of<E> auto && edges)const {
                std::vector<WE> weighted;
                std::ranges::transform(edges,std::back_inserter(weighted),[this](const E & e){return toWeightedEdge(e);});
                return weighted;
            }

        public:
            WeightedGraphDecorator():Base(){}

            WeightedGraphDecorator(adj_container_type && adjContainer):Base(std::move(adjContainer)){}

            WeightedGraphDecorator(util::forward_range_of<N> auto & nodes):Base(nodes){};

            WeightedGraphDecorator(util::forward_range_of<N> auto && nodes):Base(nodes){};
          
            bool addEdge(const WE & edge){
                auto withoutWeight = edge.unweighted();
                if(this->g.addEdge(withoutWeight)){
                    if(not weightMap.try_emplace(withoutWeight,edge.getWeight()).second){
                        weightMap[withoutWeight] = edge.getWeight();
                    }
                    return true;
                }
                return false;
            } 

            void addEdges(util::forward_range_of<WE> auto && edges) {
                std::ranges::for_each(edges,[this]( auto && edge){this->addEdge(edge);});
            }

            bool addEdge( const N & from, const N & to){
                return Base::addEdge(from,to);
            }

            bool addEdge( N && from, N && to){
                return Base::addEdge(from,to);
            }

            bool addEdge( const N & from,  const N & to, const A & annotation){
                auto e = E(from,to);
                if (Base::addEdge(e)) {
                    if(not weightMap.try_emplace(e,annotation).second){
                        weightMap[e] = annotation;
                    }
                    return true;
                }
                return false;
            }

            bool addEdge( N && from,  N && to, const A & annotation){
                auto e = E(from,to);
                if (Base::addEdge(e)){
                    if(not weightMap.try_emplace(E(from,to),annotation).second){
                        weightMap[e] = annotation;
                    }
                    return true;
                }
                return false;
            }

            bool containsEdge(const WE & edge)const{
                auto unweighted = edge.unweighted();
                return Base::containsEdge(unweighted) && toWeightedEdge(unweighted).getWeight() == edge.getWeight();
            }

            bool containsEdge(const N & from, const N & to){
                return Base::containsEdge(from,to);
            }

            bool containsEdge(const N & from, const N & to, const A & annotation){
                return containsEdge(WE(from,to,annotation));
            }

            inline WE makeEdge(const N & from, const N & to) const noexcept{
                return toWeightedEdge(this->g.makeEdge(from,to));
            }

            inline static WE makeWeightedEdge(const N & from, const N & to) {
                return WE(from,to);
            }

            inline WE makeEdge(const N & from, const N & to, const A & annotation){
                return WE(from,to,annotation);
            }

            inline static WE makeWeightedEdge(const N & from, const N & to, const A & annotation){
                return WE(from,to,annotation);
            }

            void removeEdge(const N & from, const N & to){
                E e {from,to};
                if (weightMap.contains(e)){
                    weightMap.erase(e);
                }
                Base::removeEdge(from,to);
            }

            /**
             * Remove edge if weights are equal or if no weight is stored
            */
            void removeEdge(const WE & e){
                auto unweighted = e.unweighted();
                if(not weightMap.contains(unweighted) && WE(unweighted).getWeight()==e.getWeight()){
                    Base::removeEdge(unweighted);
                }else if(weightMap.contains(unweighted) && weightMap.at(unweighted) == e.getWeight()) {
                    weightMap.erase(unweighted);
                    Base::removeEdge(unweighted);
                }
            }

            inline auto getEdges() const {
                return toWeightedEdges(this->g.getEdges());
            }

            inline auto getOutboundEdges(const N & node)const{
                return toWeightedEdges(this->g.getOutboundEdges(node));
            }

            inline auto getInboundEdges(const N & node) const{
                return toWeightedEdges(this->g.getInboundEdges(node));
            }

            void clear(){
                weightMap.clear();
                Base::clear();
            }
    };
}
namespace fishnet::graph{
/**
 * @brief Weighted Graph Decorator type 
 * 
 * @tparam G graph type
 * @tparam A annotation type
 * @tparam W weight function type
 */
template<Graph G,Annotation A, WeightFunction<typename G::node_type,A> W>
using Weighted = __impl::WeightedGraphDecorator<G,A,W>;
}