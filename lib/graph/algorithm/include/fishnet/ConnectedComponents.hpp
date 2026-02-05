#pragma once
#include "SearchResult.hpp"
#include <unordered_map>
namespace fishnet::graph{

/**
 * @brief Search Result for Connected Components.
 * @tparam N node type
 * @tparam Hash hasher type on nodes
 * @tparam Equal comparator type on nodes
 */
template<typename N,util::HashFunction<N> Hash=std::hash<N>, fishnet::util::BiPredicate<N> Equal = std::equal_to<N>>
class ConnectedComponents: public SearchResult<ConnectedComponents<N,Hash,Equal>,N,Hash,Equal>
{
protected:
    std::vector<std::vector<N>> components;
    size_t index;
    int openClosedCounter;

    virtual void handleClose(){
        return;
    }
    
public:
    using node_type = N;
    ConnectedComponents(): SearchResult<ConnectedComponents<N,Hash,Equal>,N,Hash,Equal>(){
        this->index = 0;
        this->openClosedCounter = 0;
        this->components = std::vector<std::vector<N>>();
    }

    void onOpen(const N & node) noexcept {
        // if openClosedCounter == 0, the BFS search from single vertex is finished, revealing the complete connected component
        if(openClosedCounter == 0) { 
            this->components.push_back(std::vector<N>());
        }
        this->openClosedCounter++;
        this->components[this->index].push_back(node);
    }


    void onEdge(const N & from, const N & to) noexcept{
        return;
    }

    void onClose(const N & node) noexcept {
        this->openClosedCounter--;
        // if openClosedCounter <= 0, the BFS search from single vertex is finished, revealing the complete connected component
        if(this->openClosedCounter <= 0) {
            handleClose(); // utilized by concurrent implementation, to fill the queue with components incrementally
            this->index++;
        }
    }

    bool stop() const  noexcept{
        return false;
    }

    std::vector<std::vector<N>> get() const noexcept{
        return this->components;
    }

    /**
     * @brief Get connected components as a Map: ID -> std::vector<N>
     * 
     * @return std::unordered_map<size_t,std::vector<N>>
     */
    std::unordered_map<size_t,std::vector<N>> getAsMap() & noexcept {
        std::unordered_map<size_t,std::vector<N>> map;
        size_t component_number = 0;
        for(const auto & component : components) {
            map.try_emplace(component_number,component);
            component_number++;
        }
        return map;
    }
    /**
     * @brief Get connected components as a Map: ID -> std::vector<N>
     * 
     * @return std::unordered_map<size_t,std::vector<N>>
     */
    std::unordered_map<size_t,std::vector<N>> getAsMap() && noexcept {
        std::unordered_map<size_t,std::vector<N>> map;
        size_t component_number = 0;
        for(auto && component : components) {
            map.try_emplace(component_number,std::move(component));
            component_number++;
        }
        return map;
    }

        /**
     * @brief Get connected components as a Map: Node -> component-id
     * 
     * @return std::unordered_map<N,int,Hash,Equal> storing for each node the component id as a value
     */
    std::unordered_map<N,int,Hash,Equal> nodeMap() const noexcept {
        std::unordered_map<N,int,Hash,Equal> map;
        for(size_t component_number=0; component_number < components.size(); component_number++) {
            for(auto & node : components[component_number]) {
                map.try_emplace(node,component_number);
            }
        }
        return map;
    }

    ~ConnectedComponents() = default;
};
}