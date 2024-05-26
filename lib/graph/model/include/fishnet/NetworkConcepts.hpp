#pragma once
#include <fishnet/UtilConcepts.hpp>
#include <fishnet/NumericConcepts.hpp>
namespace fishnet::graph{

template<typename A>
concept Annotation = fishnet::math::Number<A>;

template<typename N>
concept Node = std::equality_comparable<N>;

template<typename NodeType,typename N>
concept UniversalNodeRef = std::same_as<N,std::remove_cvref_t<NodeType>>;

template<typename F, typename N, typename A>
concept WeightFunction= Node<N> && Annotation<A> && util::BiFunction<F,N,N,A>;

template<typename P, typename N>
concept NodeBiPredicate = Node<N> && util::BiPredicate<P,N>;

template<typename F, typename N>
concept NodeBiOperator = Node<N> && util::BiOperator<F,N>;

}