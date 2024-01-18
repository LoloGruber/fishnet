// #pragma once
// #include "Edge.h"
// namespace graph{
// template<Node N, Annotation A,HashFunction<N> N_Hash,NodeBiPredicate<N> N_Equal> 
// class Edge<N,A,true,N_Hash,N_Equal>{
// private:
//     N  from;
//     N  to;
//     std::optional<A> weight;
//     N_Equal equalityFunction = N_Equal();
    
// public:
//     Edge(const N & from, const N & to): from(from),to(to),weight(std::nullopt) {};
//     Edge(const N & from, const N & to,const A & weight): from(from),to(to),weight(std::optional<A>{weight}){};
//     Edge(const N & from, const N & to, const std::optional<A> & optWeight): from(from),to(to),weight(optWeight){};
//     Edge(const Edge<N,A,false,N_Hash,N_Equal> & edge) :from(edge.getFrom()),to(edge.getTo()),weight(edge.getWeight()){};
//     Edge(const Edge<N,A,true,N_Hash,N_Equal> & edge) :from(edge.getFrom()),to(edge.getTo()),weight(edge.getWeight()){};

//     const N & getFrom() const{
//         return this->from;
//     }

//     const N & getTo() const {
//         return this->to;
//     }

//     const std::optional<A> & getWeight() const {
//         return this->weight;
//     }


//     bool operator==(const Edge<N,A,true,N_Hash,N_Equal> & other) const {
//         return this->equalityFunction(this->from,other.getFrom()) && this->equalityFunction(this->to, other.getTo());
//     }
//     bool operator==(const Edge<N,A,false,N_Hash,N_Equal> & other) const {
//         return false;
//     }

// };
// template<Node N, Annotation A, HashFunction<N> Hash = std::hash<N>, NodeBiPredicate<N> Equal = std::equal_to<N>>
// using DirectedEdge = Edge<N,A,true,Hash,Equal>;
// }

// namespace std{
//     template<typename N, typename A, typename N_Hash,typename N_Equal>
//     struct hash<graph::Edge<N,A,true,N_Hash,N_Equal>>{
//         size_t operator()(const graph::DirectedEdge<N,A,N_Hash,N_Equal> edge) const {
//             std::size_t hashFrom = N_Hash()(edge.getFrom());
//             std::size_t hashTo = N_Hash()(edge.getTo()) ;
//             return hashFrom ^ (hashTo << 3);
//         }
//     };
// }