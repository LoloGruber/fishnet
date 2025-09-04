#include "AdjacencyContainerDecorator.hpp"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fishnet/PathHelper.h>
#include <fstream>
#include <string_view>

namespace fishnet::graph{

template<typename BaseContainer> requires AdjacencyContainer<BaseContainer,typename BaseContainer::node_type>
class JSONFileAdjacency : public AdjacencyContainerDecorator<BaseContainer>{
private:
    constexpr static std::string_view NODES_KEY = "nodes";
    constexpr static std::string_view EDGES_KEY = "edges";
    constexpr static std::string_view NODE_ID_KEY = "id";
    constexpr static std::string_view NODE_DATA_KEY = "data";
    using N = typename BaseContainer::node_type;
    using Base = AdjacencyContainerDecorator<BaseContainer>;
    std::filesystem::path filePath;
    util::UnaryFunction_t<N,nlohmann::json> toJson;
    util::UnaryFunction_t<nlohmann::json,N> fromJson;

    void load() {
        std::ifstream file(filePath);
        if(not file.is_open() || std::filesystem::is_empty(filePath)){
            return;
        }
        nlohmann::json j;
        file >> j;
        file.close();
        if(not j.contains(NODES_KEY) or not j.contains(EDGES_KEY)){
            throw std::runtime_error("Invalid JSON format: missing nodes or edges key");
        }
        std::unordered_map<size_t,N> idToNodeMap;
        for(const auto & jsonNode: j[NODES_KEY]){
            if(not jsonNode.contains(NODE_ID_KEY) or not jsonNode.contains(NODE_DATA_KEY)){
                throw std::runtime_error("Invalid JSON format: node missing id or data key");
            }
            size_t nodeID = jsonNode[NODE_ID_KEY].get<size_t>();
            N node = fromJson(jsonNode[NODE_DATA_KEY]);
            this->addNode(std::move(node));
            idToNodeMap.emplace(nodeID, node);
        }
        for(const auto & jsonEdge: j[EDGES_KEY]){
            if(not jsonEdge.contains("from") or not jsonEdge.contains("to")){
                throw std::runtime_error("Invalid JSON format: edge missing from or to key");
            }
            size_t fromID = jsonEdge["from"].get<size_t>();
            size_t toID = jsonEdge["to"].get<size_t>();
            if(not idToNodeMap.contains(fromID) or not idToNodeMap.contains(toID)){
                throw std::runtime_error("Invalid JSON format: edge references unknown node id");
            }
            this->addAdjacency(idToNodeMap.at(fromID), idToNodeMap.at(toID));
        }
    }

    void save() const noexcept {
        nlohmann::json j;
        j[NODES_KEY] = nlohmann::json::array();
        size_t ID = 0;
        std::unordered_map<N,size_t,typename BaseContainer::hash_function,typename BaseContainer::equality_predicate> nodeToIDMap;
        for(const auto & node: this->nodes()){
            nlohmann::json jsonNode;
            size_t nodeID = ID++;
            nodeToIDMap[node] = nodeID;
            jsonNode[NODE_ID_KEY] = nodeID;
            jsonNode[NODE_DATA_KEY] = toJson(node);
            j[NODES_KEY].push_back(jsonNode);
        }
        j[EDGES_KEY] = nlohmann::json::array();
        for(const auto &[from,to]: this->getAdjacencyPairs()){
            nlohmann::json edge;
            edge["from"] = nodeToIDMap.at(from);
            edge["to"] = nodeToIDMap.at(to);
            j[EDGES_KEY].push_back(edge);
        }
        std::ofstream file(filePath);
        if(file.is_open()){
            file << j.dump();
            file.close();
        }
    }

public: 

    JSONFileAdjacency(BaseContainer && baseContainer, const std::filesystem::path  &  filePath, util::UnaryFunction<N, nlohmann::json> auto toJson, util::UnaryFunction<nlohmann::json, N> auto fromJson)
        : AdjacencyContainerDecorator<BaseContainer>(std::move(baseContainer)), filePath(fishnet::util::PathHelper::absoluteCanonical(filePath)), toJson(std::move(toJson)), fromJson(std::move(fromJson)) {
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


