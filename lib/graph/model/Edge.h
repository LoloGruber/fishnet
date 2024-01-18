#ifndef Graph_Edge_H
#define Graph_Edge_H
#include "NetworkConcepts.h"
#include <concepts>

namespace fishnet::graph{

/**
 * Minimal Interface of an Edge
 *      - construct with two nodes
 *      - get endpoints
 *      - compare if equal to other edge
*/
template<typename E, typename N=E::node_type>
concept Edge = requires(const E & e, const E & o, const N & cNodeRef /*, N&& nodeRval*/){
    {E(cNodeRef,cNodeRef)};
    /*{E(nodeRval,nodeRval)};*/
    {E::isDirected()} -> std::convertible_to<bool>;
    {e.getFrom()} -> std::same_as<const N &>;
    {e.getTo()} -> std::same_as<const N &>;
    {e == o} -> std::convertible_to<bool>;
    {e.hash()} -> std::convertible_to<size_t>;
    typename E::node_type;
};

template<typename E, typename N= E::node_type, typename A=E::annotation_type, typename W=E::weight_function>
concept WeightedEdge = Edge<E,N> && WeightFunction<W,N,A> && requires (const E & e, const N & cNodeRef, const A & constARef, A&& ARval ){
    {E(cNodeRef,cNodeRef,constARef)};
    {E(cNodeRef,cNodeRef,ARval)};
    {e.getWeight()} -> std::convertible_to<A>;
    {e.unweighted()} -> std::convertible_to<E>;
    typename E::annotation_type;
    typename E::weight_function;
};


namespace __impl{
    using namespace graph;

    template<Node N,bool Directed, util::HashFunction<N> Hash = std::hash<N>, NodeBiPredicate<N> Equal = std::equal_to<N>> 
    class BaseEdge {
        protected:
            N from;
            N to;
            static inline Equal eq = Equal();
            static inline Hash hasher = Hash();
        public:
            using node_type = N;

            
            constexpr static bool isDirected(){
                return Directed;
            }

            BaseEdge(const N & from, const N & to):from(from),to(to){}

            const N & getFrom()const {
                return from;
            }

            const N & getTo() const{
                return to;
            }

            bool operator==(  const BaseEdge<N,false,Hash,Equal> & other)const{
                if constexpr(not Directed){
                    return (eq(from, other.getFrom()) and eq(to,other.getTo())) or
                    (eq(from, other.getTo()) and eq(to, other.getFrom()));
                }
                return false;
            }

            bool operator==(const BaseEdge<N,true,Hash,Equal> & other) const {
                if constexpr(Directed){
                    return eq(from,other.getFrom()) && eq(to, other.getTo());
                }
                return false;
            }

            size_t inline hash()const{
                if constexpr(Directed){
                    size_t hashFrom = hasher(from);
                    size_t hashTo = hasher(to);
                    constexpr size_t shift = sizeof(size_t) *4;
                    return (hashFrom << shift) | hashTo;
                }else{
                    return hasher(from) ^ hasher(to);
                }
            }
    };



    
    template<Edge E,Annotation A,WeightFunction<typename E::node_type,A> W>
        class WeightedEdgeDecorator: public E{
        private:
            A weight;
            using N=typename E::node_type;

        public:
            using annotation_type=A;
            using weight_function=W;

            WeightedEdgeDecorator(const N & from, const N & to, const A & weight): E(from,to),weight(weight){};

            WeightedEdgeDecorator(const N & from, const N & to):E(from,to){
                this->weight = W()(from,to);
            }

            WeightedEdgeDecorator(const N & from, const N & to, A &&weight): E(from,to),weight(std::move(weight)){};

            WeightedEdgeDecorator( E && edge ):WeightedEdgeDecorator(edge.getFrom(),edge.getTo()){}
            WeightedEdgeDecorator(const E & edge ):WeightedEdgeDecorator(edge.getFrom(),edge.getTo()){}

            void setWeight(const A & weight){
                this->weight = weight;
            }

            const A& getWeight()const{
                return weight;
            }

            E inline unweighted()const {
                return E(this->getFrom(),this->getTo());
            }

            bool operator==(const WeightedEdgeDecorator<E,A,W> & other) const {
                return this->unweighted() == other.unweighted() && this->weight == other.getWeight();
            }

            size_t inline hash() const{
                return this->unweighted().hash() + std::hash<A>{}(weight);
            }
    };
}

template<Node N, util::HashFunction<N> Hash = std::hash<N>, NodeBiPredicate<N> Equal = std::equal_to<N>>
using UndirectedEdge = __impl::BaseEdge<N,false,Hash,Equal>;

template<Node N, util::HashFunction<N> Hash = std::hash<N>, NodeBiPredicate<N> Equal = std::equal_to<N>>
using DirectedEdge = __impl::BaseEdge<N,true,Hash,Equal>;

template<Edge E, Annotation A , WeightFunction<typename E::node_type,A> W>
using WeightEdge = __impl::WeightedEdgeDecorator<E,A,W>;


}



namespace std{
    using namespace fishnet::graph;
    template<Edge E>
    struct hash<E>{
        size_t operator()(const E & edge) const noexcept {
            return edge.hash();
        }
    };
};
#endif




