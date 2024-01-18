#pragma once
#include <ranges>
#include <vector>
#include <unordered_map>
#include "UtilConcepts.hpp"
#include <algorithm>
#include "PairSet.h"

namespace fishnet::graph{
template<typename N, util::HashFunction<N> Hash=std::hash<N>, util::BiPredicate<N> Equal=std::equal_to<N>,typename NodeRange = std::vector<N>>
class AdjacencyMap{
private:
    using Map = std::unordered_map<N, NodeRange,Hash,Equal>;
    Map map= Map();
    const static inline Equal eq=Equal();
    const static inline Hash hash = Hash();
    const static inline NodeRange EMPTY = NodeRange(); // returning views::empty<N> would not compile with gcc 11.3
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

    bool inline addNode(N & node){
        return map.try_emplace(node,NodeRange()).second;
    }

    bool inline addNode(N && node){
        return map.try_emplace(node,NodeRange()).second;
    }

    void inline removeNode(const N& node){
        if(contains(node)){
            for(auto const& neighbour : adjacency(node)){
                removeAdjacency(neighbour,node);
            }
            map.erase(node);
        }
    }

    void removeAdjacency(const N & from, const N & to){
        if(map.contains(from)) {
             auto [begin,end] = std::ranges::remove_if(map.at(from),[this,&to](N & element ){return eq(element,to);});
             map.at(from).erase(begin,end);
        }
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

    // auto getAdjacencyPairs() const noexcept {
    //     std::vector<std::pair<N,N>> adjacencies;
    //     for(const auto & [node,neighbours]: map){
    //         for(const auto & neighbour : neighbours) {
    //             adjacencies.emplace_back(std::make_pair(node,neighbour));
    //         }
    //     }
    //     return adjacencies;
    // }

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