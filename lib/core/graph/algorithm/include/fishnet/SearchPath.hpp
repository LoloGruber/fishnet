#pragma once
#include "SearchResult.hpp"
#include <optional>
#include <algorithm>
#include <fishnet/Edge.hpp>
namespace fishnet::graph{
/**
 * @brief Search Path implementation
 * 
 * @tparam E edge type
 * @tparam std::hash<typename E::node_type> hasher on node type of the edge type
 * @tparam std::equal_to<typename E::node_type> comparator on node type of the edge type
 */
template<Edge E,util::HashFunction<typename E::node_type> Hash = std::hash<typename E::node_type>,NodeBiPredicate<typename E::node_type> Equal = std::equal_to<typename E::node_type>>
class SearchPath : public SearchResult<SearchPath<E,Hash,Equal>,typename E::node_type,Hash,Equal>
{
private:
    using N = E::node_type;
    N goal;
    bool found;
    Equal eq = Equal();
    std::unordered_map<N,N> predecessor;
public:
    using node_type =  N;

    SearchPath(const N & goal): SearchResult<SearchPath<E,Hash,Equal>,N,Hash,Equal>(),goal(goal),found(false){};

    void onOpen(const N & node) {
        if(eq(node,goal)) {
            found = true;
        }
    }

    void onClose(const N & node) {
        if(eq(node,goal)) {
            found = true;
        }
    }

    void onEdge(const N & from, const N & to){
        predecessor.insert_or_assign(to,from);
    }

    bool stop() const {
        return this->found;
    }

    std::optional<std::vector<N>> get() const{
        if(not found) 
            return std::nullopt;
        std::vector<N> nodes;
        nodes.push_back(goal);
        while(this->predecessor.contains(nodes.back())) {
            nodes.push_back(this->predecessor.at(nodes.back()));
        }
        std::reverse(nodes.begin(),nodes.end());
        return std::optional<std::vector<N>>{nodes};
    }
    ~SearchPath() = default;
};
}