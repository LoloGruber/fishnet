#include "AdjacencyContainerDecorator.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fishnet/PathHelper.h>
#include <fstream>

namespace fishnet::graph{

template<typename BaseContainer> requires AdjacencyContainer<BaseContainer,typename BaseContainer::node_type>
class JSONFileAdjacency : public AdjacencyContainerDecorator<BaseContainer,N>{
private:
    using N = typename BaseContainer::node_type;
    using Base = AdjacencyContainerDecorator<BaseContainer,N>;
    std::filesystem::path filePath;
    util::UnaryFunction_t<N,nlohmann::json> toJson;
    util::UnaryFunction_t<nlohmann::json,N> fromJson;

    void load() noexcept{
        
    }

    void save() const noexcept {
        nlohmann::json ;
        j["nodes"] = nlohmann::json::array();
        size_t ID = 0;
        std::unordered_map<N,size_t,typename BaseContainer::hash_function,typename BaseContainer::equality_predicate> nodeToIDMap;
        for(const auto & node: this->nodes()){
            nlohmann::json jsonNode;
            size_t nodeID = ID++;
            nodeToIDMap[node] = nodeID;
            jsonNode["id"] = nodeID;
            jsonNode["data"] = toJson(node);
            j["nodes"].push_back(jsonNode);
        }
        j["edges"] = nlohmann::json::array();
        for(const auto &[from,to]: this->getAdjacencyPairs()){
            nlohmann::json edge;
            edge["from"] = nodeToIDMap.at(from);
            edge["to"] = nodeToIDMap.at(to);
            j["edges"].push_back(edge);
        }
        std::ofstream file(filePath);
        if(file.is_open()){
            file << j.dump(4);
            file.close();
        }
    }

public: 

    JSONFileAdjacency(BaseContainer && baseContainer, const std::filesystem::path  &  filePath, util::UnaryFunction<N, nlohmann::json> toJson, util::UnaryFunction<nlohmann::json, N> fromJson)
        : AdjacencyContainerDecorator<BaseContainer,N>(std::move(baseContainer)), filePath(fishnet::util::PathHelper::absoluteCanonical(filePath)), toJson(std::move(toJson)), fromJson(std::move(fromJson)) {
        if(not this->filePath.has_extension() or this->filePath.extension() != ".json"){
            throw std::invalid_argument("File path must have .json extension");
        }
        load();
    }

    ~JSONFileAdjacency(){
        save();
    }
};
}


