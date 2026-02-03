#pragma once
#include <fstream>
#include <string_view>
#include <vector>
#include <cstdint>
#include <fishnet/FunctionalConcepts.hpp>
#include "AdjacencyContainerDecorator.hpp"

namespace fishnet::graph{

template<typename BaseContainer> requires AdjacencyContainer<BaseContainer,typename BaseContainer::node_type>
class BinaryAdjacency : public AdjacencyContainerDecorator<BaseContainer>{
private:
    using N = typename BaseContainer::node_type;
    using Base = AdjacencyContainerDecorator<BaseContainer>;
    util::UnaryFunction_t<N,std::vector<uint8_t>> toBinary;
    util::UnaryFunction_t<std::vector<uint8_t>,N> fromBinary;
public: 

    BinaryAdjacency(BaseContainer && baseContainer, 
                        util::UnaryFunction<N, std::vector<uint8_t>> auto toBinary, 
                        util::UnaryFunction<std::vector<uint8_t>, N> auto fromBinary)
        : AdjacencyContainerDecorator<BaseContainer>(std::move(baseContainer)), toBinary(std::move(toBinary)), fromBinary(std::move(fromBinary)) {}

    std::vector<uint8_t> get() const noexcept {
        std::vector<uint8_t> result;
        
        auto write_bytes = [&](const void* data, size_t size) {
            const uint8_t* bytes = static_cast<const uint8_t*>(data);
            result.insert(result.end(), bytes, bytes + size);
        };

        auto write_u64 = [&](uint64_t value) {
            write_bytes(&value, sizeof(uint64_t));
        };

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
        write_u64(numNodes);
        
        // Write nodes
        for(const auto & [id, data] : nodeData){
            write_u64(id);
            uint64_t dataSize = data.size();
            write_u64(dataSize);
            write_bytes(data.data(), dataSize);
        }
        
        // Collect edges
        auto edges = this->getAdjacencyPairs();
        uint64_t numEdges = fishnet::util::size(edges);
        
        // Write number of edges
        write_u64(numEdges);
        
        // Write edges
        for(const auto &[from,to]: edges){
            uint64_t fromID = nodeToIDMap.at(from);
            uint64_t toID = nodeToIDMap.at(to);
            write_u64(fromID);
            write_u64(toID);
        }
        
        return result;
    }

    void load(const std::vector<uint8_t> & data) {
        if(data.empty()){
            return;
        }

        size_t offset = 0;
        auto ensure_bytes = [&](size_t need){
            if(offset + need > data.size()){
                throw std::runtime_error("Binary data truncated");
            }
        };

        auto read_u64 = [&]() -> uint64_t {
            ensure_bytes(sizeof(uint64_t));
            uint64_t v = 0;
            for(size_t i = 0; i < sizeof(uint64_t); ++i){
                v |= static_cast<uint64_t>(data[offset + i]) << (8 * i);
            }
            offset += sizeof(uint64_t);
            return v;
        };

        // Read number of nodes
        uint64_t numNodes = read_u64();

        std::unordered_map<uint64_t,N> idToNodeMap;
        idToNodeMap.reserve(numNodes);

        // Read nodes
        for(uint64_t i = 0; i < numNodes; ++i){
            uint64_t nodeID = read_u64();
            uint64_t dataSize = read_u64();
            if(dataSize > data.size() - offset){
                throw std::runtime_error("Binary data truncated while reading node payload");
            }
            std::vector<uint8_t> nodeBytes(data.begin() + offset, data.begin() + offset + static_cast<size_t>(dataSize));
            offset += static_cast<size_t>(dataSize);

            N node = fromBinary(nodeBytes);
            this->addNode(node);
            idToNodeMap.emplace(nodeID, std::move(node));
        }

        // Read number of edges
        uint64_t numEdges = read_u64();

        // Read edges
        for(uint64_t i = 0; i < numEdges; ++i){
            uint64_t fromID = read_u64();
            uint64_t toID = read_u64();
            if(not idToNodeMap.contains(fromID) or not idToNodeMap.contains(toID)){
                throw std::runtime_error("Binary data format error: edge references unknown node id");
            }
            this->addAdjacency(idToNodeMap.at(fromID), idToNodeMap.at(toID));
        }
    }
};
} // namespace fishnet::graph