#pragma once
#include <ranges>
#include <vector>
#include <unordered_map>
#include <fishnet/UtilConcepts.hpp>
#include <algorithm>
#include <fishnet/PairSet.hpp>

namespace fishnet::graph{
/**
 * @brief Implementation of an Adjacency Map.
 * Implements AdjacencyContainer
 * @tparam N node type
 * @tparam Hash hasher type on N
 * @tparam Equal comparator type on N
 * @tparam NodeRange range type for adjacency list
 */
template<typename N, util::HashFunction<N> Hash=std::hash<N>, util::BiPredicate<N> Equal=std::equal_to<N>,typename NodeRange = std::vector<N>>
class AdjacencyMap{
private:
    using Map = std::unordered_map<N, NodeRange,Hash,Equal>;
    Map map= Map();
    const static inline Equal eq=Equal();
    const static inline Hash hash = Hash();
    const static inline NodeRange EMPTY = NodeRange(); // returning views::empty<N> would not compile with gcc 11.3 -> return view on empty node range
public:

    using equality_predicate = Equal;
    using hash_function = Hash;
    void addAdjacency(N & from, N & to){
        map.try_emplace(from,NodeRange());
        map.try_emplace(to,NodeRange());
        map[from].push_back(to);
    }

    void addAdjacency(N && from, N && to){
        map.try_emplace(to,NodeRange());
        map.try_emplace(from,NodeRange());
        map[from].emplace_back(to);
    }

    void addAdjacencies(util::forward_range_of<std::pair<N,N>> auto && pairs){
        std::ranges::for_each(pairs,[this](auto & pair){addAdjacency(pair.first,pair.second);});
    }

    bool inline addNode(N & node){
        return map.try_emplace(node,NodeRange()).second;
    }

    bool inline addNode(N && node){
        return map.try_emplace(node,NodeRange()).second;
    }

    bool addNodes(util::forward_range_of<N> auto && nodes) {
        return std::ranges::fold_left(nodes,false,[this](bool prev, auto n){return addNode(n) & prev;});
    }

    void inline removeNode(const N& node){
        if(contains(node)){
            for(auto const& neighbour : adjacency(node)){
                removeAdjacency(neighbour,node);
            }
            map.erase(node);
        }
    }

    void removeNodes(util::forward_range_of<N> auto && nodes){
        std::ranges::for_each(nodes,[this](auto && n){removeNode(n);});
    }

    void removeAdjacency(const N & from, const N & to){
        if(map.contains(from)) {
             auto [begin,end] = std::ranges::remove_if(map.at(from),[this,&to](N & element ){return eq(element,to);});
             map.at(from).erase(begin,end);
        }
    }

    void removeAdjacencies(util::forward_range_of<std::pair<N,N>> auto && adjacencies) {
        std::ranges::for_each(adjacencies,[this](auto && pair){removeAdjacency(pair.first,pair.second);});
    }

    bool inline contains(const N & node) const noexcept{
        return map.contains(node);
    }

    bool hasAdjacency(const N & from, const N & to) const noexcept{
        return contains(from) && std::ranges::any_of(map.at(from),[this,&to](const N & element ){return eq(to,element); });
    }

    auto adjacency(const N & node) const noexcept{
        if(contains(node)) {
            return std::views::all(map.at(node));
        }
        return std::views::all(EMPTY);
    }

    auto nodes() const noexcept {
        return std::views::keys(map);
    }

    auto getAdjacencyPairs() const noexcept {
        return std::views::all(map) 
            | std::views::transform([](const auto & pair){
                const auto & [from,neighbours] = pair;
                return std::views::transform(neighbours,[from](const auto & to){return std::make_pair(from,to);});}) 
            | std::views::join; 
    }

    void clear() noexcept {
        map.clear();
    }
};

}