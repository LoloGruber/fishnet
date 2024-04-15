#pragma once
#include "Graph.hpp"
#include "Edge.hpp"
#include "SimpleGraph.hpp"
#include "NetworkConcepts.hpp"
#include "GraphDecorator.hpp"
#include <unordered_map>

namespace fishnet::graph{

template<typename GraphImpl, typename E= GraphImpl::edge_type, typename N= E::node_type>
concept WeightedGraph = Node<N> && WeightedEdge<E> && Graph<GraphImpl,E,N>;

namespace __impl {
    using namespace graph;

    template<class G,Annotation A, WeightFunction<typename G::node_type,A> W> //requires Graph<G<typename G::edge_type,typename G::node_type>,typename G::edge_type,typename G::node_type>
    class WeightedGraphDecorator: public GraphDecorator<WeightedGraphDecorator<G,A,W>,G,WeightEdge<typename G::edge_type,A,W>>{
        private:
            using E = G::edge_type;
            using WE= WeightEdge<E,A,W>;
            using N = G::node_type;
            using WeightMap = std::unordered_map<E,A>;
            using Base = GraphDecorator<WeightedGraphDecorator<G,A,W>,G,WE>;
            WeightMap weightMap;
        public:
            using edge_type=WE;
            using adj_container_type = G::adj_container_type;
            static_assert(! WeightedEdge<E, typename E::node_type, A,W>);
        private:
            inline WE toWeightedEdge(const E & e)const{
                if (weightMap.contains(e)){
                    return WE(e.getFrom(),e.getTo(),weightMap.at(e));
                }
                return WE(e);
            }

            std::vector<WE> toWeightedEdges(util::forward_range_of<E> auto && edges)const {
                std::vector<WE> weighted;
                std::ranges::transform(edges,std::back_inserter(weighted),
                [this](const E & e){return toWeightedEdge(e);});
                return weighted;
            }

        public:
            WeightedGraphDecorator():Base(){}

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

            bool addEdge( N & from,  N & to){
                return Base::addEdge(from,to);
            }

            bool addEdge( N && from, N && to){
                return Base::addEdge(from,to);
            }

            // bool addEdge(const E & e){
            //     return Base::addEdge(e);
            // }

            bool addEdge( N & from,  N & to, const A & annotation){
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
                auto unweight = edge.unweighted();
                return Base::containsEdge(unweight) && toWeightedEdge(unweight).getWeight() == edge.getWeight();
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
                auto unweight = e.unweighted();
                if(not weightMap.contains(unweight) && WE(unweight).getWeight()==e.getWeight()){
                    Base::removeEdge(unweight);
                }else if(weightMap.contains(unweight) && weightMap.at(unweight) == e.getWeight()) {
                    weightMap.erase(unweight);
                    Base::removeEdge(unweight);
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
template<Graph G,Annotation A, WeightFunction<typename G::node_type,A> W>
using Weighted = __impl::WeightedGraphDecorator<G,A,W>;

}