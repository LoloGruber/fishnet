#include <future>
#include <CLI/CLI.hpp>
#include <fishnet/Graph.hpp>
#include <fishnet/Task.hpp>
#include "BinarySettlementGraphAdjacency.hpp"

struct ClusterWorkload{
    std::vector<std::string> files;
    std::filesystem::path graphBinaryFile;
};

class GraphComponents : public Task {
private:
    std::vector<std::filesystem::path> binGraphFiles;

    auto readInput() {
        auto fileIdToPathMap = std::unordered_map<FileReference, std::filesystem::path>{};
        auto graph = fishnet::graph::GraphFactory::UndirectedGraph<ProxySettlement>();
        for(const auto & binGraphFile : binGraphFiles) {
            auto subGraphAdjacency = ReadingBinarySettlementGraphAdjacency<ProxySettlement>(
                binGraphFile,
                ProxySettlementDeserializer{}
            );
            for(auto && [from,to]: subGraphAdjacency.getAdjacencyPairs()) {
                graph.addEdge(std::move(from), std::move(to));
            }
            graph.addNodes(std::move(subGraphAdjacency.nodes()));
            for(auto && [fileID, path] : subGraphAdjacency.getFileRefToPathMap()) {
                fileIdToPathMap.try_emplace(fileID, std::move(path));
            }
        }
        return std::make_pair(std::move(graph), std::move(fileIdToPathMap));
    }


    std::unordered_map<size_t, std::vector<FileReference>> componentToFilesMap(const std::unordered_map<size_t, std::vector<ProxySettlement>> & componentToSettlements) {
        std::unordered_map<size_t, std::vector<FileReference>> compToFilesMap;
        for (const auto& [componentID, settlements] : componentToSettlements) {
            std::unordered_set<FileReference> fileRefsSet;
            for (const auto& settlement : settlements) {
                fileRefsSet.insert(settlement.file());
            }
            compToFilesMap[componentID] = std::vector<FileReference>(fileRefsSet.begin(), fileRefsSet.end());
        }
        return compToFilesMap;
    }

public: 
    GraphComponents(std::vector<std::filesystem::path> binGraphFiles): binGraphFiles(std::move(binGraphFiles)){}

    void run() override{
        auto [graph, fileIdToPathMap] = readInput();
        auto components = fishnet::graph::BFS::connectedComponents(graph).getAsMap();
        




    }
};



int main(int argc, char * argv[]){
    CLI::App app{"AfricapolisGraphComponents"};
    std::vector<std::string> binaryGraphFiles;
    std::string configFilename;
    app.add_option("-g,--graph-files", binaryGraphFiles, "Input binary graph files to partition into graph components")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-c,--config", configFilename, "Path to configuration file for graph components stage of Africapolis workflow")
        ->check(CLI::ExistingFile); // currently not required / used
    CLI11_PARSE(app, argc, argv);
    std::vector<std::filesystem::path> binGraphPaths;
    for(auto && fileStr : binaryGraphFiles){
        binGraphPaths.push_back(std::filesystem::path(std::move(fileStr)));
    }
    GraphComponents task(std::move(binGraphPaths));
    task.run();
    return 0;
}