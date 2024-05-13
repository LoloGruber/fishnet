#pragma once 
#include <fishnet/HashConcepts.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include <unordered_set>

namespace fishnet::util{

template<typename T, HashFunction<T> Hash = std::hash<T>>
struct OrderedPairHash{
    const static inline Hash hash_function = Hash();
    size_t operator()(const std::pair<T,T> & pair) const {
        size_t first = hash_function(pair.first);
        size_t second = hash_function(pair.second);
        constexpr size_t shift = sizeof(size_t) *4;
        return (first << shift) | second;
    }
};

template<typename T, HashFunction<T> Hash = std::hash<T>>
struct UnorderedPairHash{
    const static inline Hash hash_function = Hash();
    size_t operator()(const std::pair<T,T> & pair) const {
        return hash_function(pair.first) ^ hash_function(pair.second);
    }
};

template<typename T, BiPredicate<T> Equal = std::equal_to<T>>
struct OrderedPairEqual{
    const static inline Equal equality_predicate = Equal();
    bool operator()(const std::pair<T,T> & p1, const std::pair<T,T> & p2) const {
        return equality_predicate(p1.first,p2.first) && (equality_predicate(p1.second,p2.second));
    }
};


template<typename T, BiPredicate<T> Equal = std::equal_to<T>>
struct UnorderedPairEqual{
    const static inline Equal equality_predicate = Equal();
    bool operator()(const std::pair<T,T> & p1, const std::pair<T,T> & p2) const {
        return (equality_predicate(p1.first,p2.first) && (equality_predicate(p1.second,p2.second))) or
            (equality_predicate(p1.first,p2.second) && equality_predicate(p1.second, p2.first));
    }
};

template<typename T, HashFunction<T> Hash = std::hash<T>, BiPredicate<T> Equal = std::equal_to<T>>
using unordered_pair_set = std::unordered_set<std::pair<T,T>, UnorderedPairHash<T,Hash>, UnorderedPairEqual<T,Equal>>;

template<typename T, HashFunction<T> Hash = std::hash<T>, BiPredicate<T> Equal = std::equal_to<T>>
using ordered_pair_set = std::unordered_set<std::pair<T,T>, OrderedPairHash<T,Hash>, OrderedPairEqual<T,Equal>>;


template<class S, typename T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>>
concept PairSet = HashFunction<Hash,T> && BiPredicate<Equal,T> && (std::convertible_to<S, unordered_pair_set<T,Hash,Equal>> || std::convertible_to<S, ordered_pair_set<T,Hash,Equal>>);
}