#pragma once
#include <fishnet/FunctionalConcepts.hpp>
namespace util{

template<typename Key>
concept Hashable = requires(Key a)
{
    { std::hash<Key>{}(a) } -> std::convertible_to<std::size_t>;
};

template<typename Hasher,typename Key>
concept HashFunction = UnaryFunction<Hasher,Key,size_t>;

template<typename T>
concept Mapable = Hashable<T> && std::equality_comparable<T>;
}