#include <fstream>
#include <string_view>
#include <filesystem>
#include <vector>
#include <cstdint>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/PathHelper.h>
#include "AdjacencyContainerDecorator.hpp"

namespace fishnet::graph{

template<typename BaseContainer> requires AdjacencyContainer<BaseContainer,typename BaseContainer::node_type>
class BinaryFileAdjacency : public AdjacencyContainerDecorator<BaseContainer>{
private:
    using N = typename BaseContainer::node_type;
    using Base = AdjacencyContainerDecorator<BaseContainer>;
    std::filesystem::path filePath;
    util::UnaryFunction_t<N,std::vector<uint8_t>> toBinary;
    util::UnaryFunction_t<std::vector<uint8_t>,N> fromBinary;

    void load() {
        std::ifstream file(filePath, std::ios::binary);
        if(not file.is_open() || std::filesystem::is_empty(filePath)){
            return;
        }
        
        // Read number of nodes
        uint64_t numNodes;
        file.read(reinterpret_cast<char*>(&numNodes), sizeof(uint64_t));
        
        std::unordered_map<uint64_t,N> idToNodeMap;
        idToNodeMap.reserve(numNodes);
        
        // Read nodes
        for(uint64_t i = 0; i < numNodes; ++i){
            uint64_t nodeID;
            file.read(reinterpret_cast<char*>(&nodeID), sizeof(uint64_t));
            
            uint64_t dataSize;
            file.read(reinterpret_cast<char*>(&dataSize), sizeof(uint64_t));
            
            std::vector<uint8_t> data(dataSize);
            file.read(reinterpret_cast<char*>(data.data()), dataSize);
            
            N node = fromBinary(data);
            this->addNode(node);
            idToNodeMap.emplace(nodeID, std::move(node));
        }
        
        // Read number of edges
        uint64_t numEdges;
        file.read(reinterpret_cast<char*>(&numEdges), sizeof(uint64_t));
        
        // Read edges
        for(uint64_t i = 0; i < numEdges; ++i){
            uint64_t fromID, toID;
            file.read(reinterpret_cast<char*>(&fromID), sizeof(uint64_t));
            file.read(reinterpret_cast<char*>(&toID), sizeof(uint64_t));
            
            if(not idToNodeMap.contains(fromID) or not idToNodeMap.contains(toID)){
                throw std::runtime_error("Binary file format error: edge references unknown node id");
            }
            this->addAdjacency(idToNodeMap.at(fromID), idToNodeMap.at(toID));
        }
        
        file.close();
    }

    void save() const noexcept {
        std::ofstream file(filePath, std::ios::binary);
        if(not file.is_open()){
            return;
        }
        
        uint64_t nodeID = 0;
        std::unordered_map<N,uint64_t,typename BaseContainer::hash_function,typename BaseContainer::equality_predicate> nodeToIDMap;
        
        // Collect nodes and assign IDs
        std::vector<std::pair<uint64_t,std::vector<uint8_t>>> nodeData;
        for(const auto & node: this->nodes()){
            uint64_t id = nodeID++;
            nodeToIDMap[node] = id;
            std::vector<uint8_t> data = toBinary(node);
            nodeData.emplace_back(id, std::move(data));
        }
        
        // Write number of nodes
        uint64_t numNodes = nodeData.size();
        file.write(reinterpret_cast<const char*>(&numNodes), sizeof(uint64_t));
        
        // Write nodes
        for(const auto & [id, data] : nodeData){
            file.write(reinterpret_cast<const char*>(&id), sizeof(uint64_t));
            uint64_t dataSize = data.size();
            file.write(reinterpret_cast<const char*>(&dataSize), sizeof(uint64_t));
            file.write(reinterpret_cast<const char*>(data.data()), dataSize);
        }
        
        // Collect edges
        auto edges = this->getAdjacencyPairs();
        uint64_t numEdges = fishnet::util::size(edges);
        
        // Write number of edges
        file.write(reinterpret_cast<const char*>(&numEdges), sizeof(uint64_t));
        
        // Write edges
        for(const auto &[from,to]: edges){
            uint64_t fromID = nodeToIDMap.at(from);
            uint64_t toID = nodeToIDMap.at(to);
            file.write(reinterpret_cast<const char*>(&fromID), sizeof(uint64_t));
            file.write(reinterpret_cast<const char*>(&toID), sizeof(uint64_t));
        }
        
        file.close();
    }

public: 

    BinaryFileAdjacency(BaseContainer && baseContainer, const std::filesystem::path  &  filePath, 
                        util::UnaryFunction<N, std::vector<uint8_t>> auto toBinary, 
                        util::UnaryFunction<std::vector<uint8_t>, N> auto fromBinary)
        : AdjacencyContainerDecorator<BaseContainer>(std::move(baseContainer)), 
          filePath(fishnet::util::PathHelper::absoluteCanonical(filePath)), 
          toBinary(std::move(toBinary)), 
          fromBinary(std::move(fromBinary)) {
        if(not this->filePath.has_extension() or this->filePath.extension() != ".bin"){
            throw std::invalid_argument("File path must have .bin extension");
        }
        load();
    }

    ~BinaryFileAdjacency(){
        save();
    }
};
}
