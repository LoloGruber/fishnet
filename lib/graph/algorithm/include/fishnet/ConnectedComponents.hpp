#ifndef ConnectedComponents_H
#define ConnectedComponents_H
#include "SearchResult.hpp"
#include <unordered_map>
namespace fishnet::graph{

template<typename N,util::HashFunction<N> Hash=std::hash<N>, NodeBiPredicate<N> Equal = std::equal_to<N>>
class ConnectedComponents: public SearchResult<ConnectedComponents<N,Hash,Equal>,N,Hash,Equal>
{
protected:
    std::vector<std::vector<N>> components;
    int index;
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


    void onOpen(const N & node)  {
        if(openClosedCounter == 0) {
            this->components.push_back(std::vector<N>());
        }
        this->openClosedCounter++;
        this->components[this->index].push_back(node);
    }


    void onEdge(const N & from, const N & to){
        return;
    }


    void onClose(const N & node) {
        this->openClosedCounter--;

        if(this->openClosedCounter <= 0) {
            handleClose();
            this->index++;
        }
    }

    bool stop() const  {
        return false;
    }


    std::vector<std::vector<N>> get() const {
        return this->components;
    }

    std::unordered_map<N,int,Hash,Equal> asMap() const {
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



#endif