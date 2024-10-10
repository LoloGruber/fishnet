#pragma once
#include "MemgraphAdjacency.hpp"
#include <fishnet/AdjacencyMap.hpp>

template<DatabaseNode N>
class CachingMemgraphAdjacency: public MemgraphAdjacency<N>{
private:
    AdjacencyMap<NodeIdType> cache;
    using Base=MemgraphAdjacency<N>;
private:
    void loadEdges() noexcept {
        for(const auto & [from,to]:Base::getAdjacencyPairs()){
            cache.addAdjacency(from.key(),to.key());
        }
    }

public:
    explicit CachingMemgraphAdjacency(MemgraphClient && memgraphClient):Base(std::move(memgraphClient)){
        loadEdges();
    }

    bool addAdjacency(const N & from, const N & to) noexcept {
        bool result = Base::addAdjacency(from,to);
        if(result){
            cache.addAdjacency(from.key(),to.key());
        }
        return result;
    }

    bool addAdjacency(N && from, N && to) noexcept {
        auto fKey = from.key();
        auto tKey = to.key();
        if(Base::addAdjacency(std::move(from),std::move(to))){
            cache.addAdjacency(fKey,tKey);
            return true;
        }
        return false;
    }

    bool addAdjacencies(fishnet::util::forward_range_of<std::pair<N,N>> auto && edges) {
        std::vector<std::pair<NodeIdType,NodeIdType>> idPairs;
        idPairs.reserve(fishnet::util::size(edges));
        for(const auto & [from,to]:edges) {
            idPairs.emplace_back(from.key(),to.key());
        }
        if(Base::addAdjacencies(std::move(edges))){
            cache.addAdjacencies(std::move(idPairs));
            return true;
        }
        return false;
    }

    bool addNode(const N & node) noexcept {
        N copy = node;
        return addNode(std::move(copy));
    }

    bool addNode(N && node) noexcept {
        auto key = node.key();
        if(Base::addNode(std::move(node))){
            cache.addNode(key);
            return true;
        }
        return false;
    }

    bool addNodes(fishnet::util::forward_range_of<N> auto && nodes) noexcept {
        std::vector<NodeIdType> ids;
        ids.reserve(fishnet::util::size(nodes));
        for(const auto & node: nodes) {
            ids.push_back(node.key());
        }
        if(Base::addNodes(nodes)){
            cache.addNodes(std::move(ids));
            return true;
        }
        return false;
    }

    bool removeNode(const N & node) noexcept {
        if(Base::removeNode(node)){
            cache.removeNode(node.key());
            return true;
        }
        return false;
    }

    bool removeNodes(fishnet::util::forward_range_of<N> auto && nodes) noexcept {
        if(Base::removeNodes(nodes)){
            cache.removeNodes(std::views::transform(nodes,[](const auto & node){return node.key();}));
            return true;
        }
        return false;
    }


    bool removeAdjacency(const N & from, const N & to) noexcept {
        if(Base::removeAdjacency(from,to)){
            cache.removeAdjacency(from.key(),to.key());
            return true;
        }
        return false;
    }

    bool removeAdjacencies(fishnet::util::forward_range_of<std::pair<N,N>> auto && edges){
        if(Base::removeAdjacencies(edges)){
            cache.removeAdjacencies(std::views::transform(edges,[](const auto & pair){
                const auto & [from,to] = pair;
                return std::make_pair<NodeIdType,NodeIdType>(from.key(),to.key());
            }));
            return true;
        }
        return false;
    }

    bool contains(const N & node) const noexcept {
        return cache.contains(node.key());
    }

    bool hasAdjacency(const N & from, const N & to) const noexcept {
        return cache.hasAdjacency(from.key(),to.key());
    }

    fishnet::util::view_of<const N> auto adjacency(const N & node) const noexcept {
        return cache.adjacency(node.key()) | std::views::transform([this](auto nodeId){
            return this->keyToNodeMap.at(nodeId);
        });
    }

    fishnet::util::view_of<const N> auto nodes() const noexcept {
        return Base::nodes();
    }

    fishnet::util::view_of<std::pair<const N, const N>> auto getAdjacencyPairs() const noexcept {
        return cache.getAdjacencyPairs() | std::views::transform([this](const auto & pair){
            auto [from,to] = pair;
            return std::make_pair(this->keyToNodeMap.at(from),this->keyToNodeMap.at(to));
        });
    }

    void clear() {
        Base::clear();
        cache.clear();
    }


    template<fishnet::util::forward_range_of<ComponentReference> ComponentRange = std::vector<ComponentReference>>
    bool loadNodes(fishnet::util::forward_range_of<N> auto && nodes, ComponentRange && componentIds = {}){
        bool success = Base::loadNodes(std::forward<decltype(nodes)>(nodes),std::move(componentIds));
        if(not success) {
            return false;
        }
        loadEdges();
        return true;
    }
};
