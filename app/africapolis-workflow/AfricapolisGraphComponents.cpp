#include <fishnet/MemgraphClient.hpp>
#include <fishnet/MemgraphConnection.hpp>
#include <fishnet/TaskConfig.hpp>
#include <fishnet/GraphFactory.hpp>
#include <fishnet/BFSAlgorithm.hpp>

#include <CLI/CLI.hpp>

auto loadIDGraph( MemgraphClient const & mgClient) {
    auto nodesList = mgClient.nodes();
    auto adjMap = mgClient.edges();
    auto graph = fishnet::graph::GraphFactory::UndirectedGraph<size_t>();
    graph.addNodes(nodesList);
    for (auto &&[node, neighbours] : adjMap) {
        for (auto neighbour : neighbours) {
            graph.addEdge(node, neighbour);
        }
    }
    return graph;
}

struct ComponentReferenceHash{
    size_t operator()(const ComponentReference & ref) const {
        return asNodeIdType(ref.componentId);
    }
};

struct ComponentReferenceEqual{
    bool operator()(const ComponentReference & lhs, const ComponentReference & rhs) const {
        return lhs.componentId == rhs.componentId;
    }
};

using ComponentMap = std::unordered_map<ComponentReference, std::vector<size_t>, ComponentReferenceHash, ComponentReferenceEqual>;

ComponentMap findAndStoreComponents(const fishnet::graph::Graph auto & IDGraph, const MemgraphClient & mgClient) {
    auto components = fishnet::graph::BFS::connectedComponents(IDGraph).get();
    ComponentMap componentMap;
    for (auto && nodesOfComponent : components) {
        auto optComponent = mgClient.createComponent(nodesOfComponent);
        if (optComponent.has_value()) {
            componentMap.try_emplace(optComponent.value(), std::vector<size_t>(std::move(nodesOfComponent)));
        }
    }
    return componentMap;
}

auto distributeComponentsWorkload(){
    return 0;
}

int main(int argc, char *argv[]){
    CLI::App app{"AfricapolisGraphComponents"};
    std::string configFilename;
    std::string componentJsonFilename;
    app.add_option("-c,--config", configFilename, "Path to configuration file for graph components stage of Africapolis workflow")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outputComponentFile", componentJsonFilename, "Path to output file storing the components in json format")
        ->required()
        ->check(CLI::ExistingFile);
    CLI11_PARSE(app, argc, argv);
    MemgraphTaskConfig config{json::parse(std::ifstream(configFilename))};
    MemgraphClient mgClient = MemgraphClient(MemgraphConnection::create(config.params).value_or_throw());
    auto graph = loadIDGraph(mgClient);
    auto components = findAndStoreComponents(graph, mgClient);
    nlohmann::json jsonComponents;
    for (const auto &[componentRef, nodes] : components) {
        jsonComponents[std::to_string(componentRef.componentId)] = nodes;
    }
    std::ofstream outputFile(componentJsonFilename);
    outputFile << jsonComponents.dump(4) << std::endl;
    outputFile.close();
    return 0;
}