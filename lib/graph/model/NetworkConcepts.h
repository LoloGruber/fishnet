#ifndef NETWORK_CONCEPTS_H
#define NETWORK_CONCEPTS_H
#include "FunctionalConcepts.hpp"
#include "HashConcepts.hpp"
#include "NumericConcepts.hpp"
namespace fishnet::graph{

template<typename A>
concept Annotation = fishnet::math::Number<A>;

template<typename N>
concept Node = util::Hashable<N> && std::equality_comparable<N>;

template<typename F, typename N, typename A>
concept WeightFunction= Node<N> && Annotation<A> && util::BiFunction<F,N,N,A>;

template<typename P, typename N>
concept NodeBiPredicate = Node<N> && util::BiPredicate<P,N>;

template<typename F, typename N>
concept NodeBiOperator = Node<N> && util::BiOperator<F,N>;

}
#endif