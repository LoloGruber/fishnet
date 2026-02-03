#pragma once
#include <fstream>
#include <string_view>
#include <filesystem>
#include <vector>
#include <cstdint>
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/PathHelper.h>
#include "BinaryAdjacency.hpp"

namespace fishnet::graph{

template<typename BaseContainer> requires AdjacencyContainer<BaseContainer,typename BaseContainer::node_type>
class BinaryFileAdjacency : public BinaryAdjacency<BaseContainer>{
private:
    using N = typename BaseContainer::node_type;
    using Base = BinaryAdjacency<BaseContainer>;
    std::filesystem::path filePath;
    util::UnaryFunction_t<N,std::vector<uint8_t>> toBinary;
    util::UnaryFunction_t<std::vector<uint8_t>,N> fromBinary;

    void read() {
        std::ifstream file(filePath, std::ios::binary);
        if(not file.is_open() || std::filesystem::is_empty(filePath)){
            return;
        }

        // Read the binary data
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        // Load the data into the graph
        this->load(data);
    }

    void write() const noexcept {
        std::ofstream file(filePath, std::ios::binary);
        if(not file.is_open()){
            return;
        }
        auto data = this->get();
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();
    }

public: 

    BinaryFileAdjacency(BaseContainer && baseContainer, const std::filesystem::path  &  filePath, 
                        util::UnaryFunction<N, std::vector<uint8_t>> auto&& toBinary, 
                        util::UnaryFunction<std::vector<uint8_t>, N> auto&& fromBinary)
        : BinaryAdjacency<BaseContainer>(std::move(baseContainer),std::forward<decltype(toBinary)>(toBinary),std::forward<decltype(fromBinary)>(fromBinary)), 
          filePath(fishnet::util::PathHelper::absoluteCanonical(filePath))  
    {
        if(not this->filePath.has_extension() or this->filePath.extension() != ".bin"){
            throw std::invalid_argument("File path must have .bin extension");
        }
        read();
    }

    ~BinaryFileAdjacency(){
        write();
    }
};
}
