#include <fishnet/MemgraphClient.hpp>
#include <fishnet/MemgraphConnection.hpp>
#include <fishnet/TaskConfig.hpp>
#include <fishnet/GraphFactory.hpp>
#include <fishnet/BFSAlgorithm.hpp>
#include <fishnet/CipherQuery.hpp>
#include <fishnet/MemgraphModel.hpp>

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

struct FileReferenceHash{
    size_t operator()(const FileReference & ref) const {
        return asNodeIdType(ref.fileId);
    }
};

struct FileReferenceEqual{
    bool operator()(const FileReference & lhs, const FileReference & rhs) const {
        return lhs.fileId == rhs.fileId;
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

struct ClusterWorkload{
    size_t nodes;
    std::vector<std::string> files;
    std::vector<size_t> components;
};

void to_json(nlohmann::json & j, const ClusterWorkload & workload) {
    j = nlohmann::json{
        {"nodes", workload.nodes},
        {"files", workload.files},
        {"components", workload.components}
    };
}

auto getFiles(MemgraphConnection const & dbConnection){
    return 0;
}

auto componentIDToFileID(MemgraphConnection const & mgConnection, ComponentMap const & componentMap) {
    std::unordered_map<ComponentReference, std::vector<FileReference>, ComponentReferenceHash, ComponentReferenceEqual> componentToFilesMap;
    std::unordered_map<FileReference, std::vector<ComponentReference>, FileReferenceHash, FileReferenceEqual> fileToComponentsMap;
    if(not mgConnection.execute(
        CipherQuery("MATCH (c:Component)<-[:part_of]-(:Settlement)-[:stored]->(f:File)")
        .endl()
        .ret("DISTINCT ID(c)","ID(f)")
    )){
        throw std::runtime_error("Could not load files from database");
    }
    while(auto currentRow = mgConnection->FetchOne()){
        auto component = ComponentReference(currentRow->at(0).ValueInt());
        auto file = FileReference(currentRow->at(1).ValueInt());
        if(not componentMap.contains(component)){
            continue;
        }
        if(not fileToComponentsMap.contains(file))
            fileToComponentsMap.try_emplace(file,std::vector<ComponentReference>());
        if(not componentToFilesMap.contains(component))
            componentToFilesMap.try_emplace(component,std::vector<FileReference>());
        componentToFilesMap.at(component).push_back(file); 
        fileToComponentsMap.at(file).push_back(component);
    }
    return std::make_pair(componentToFilesMap,fileToComponentsMap);
}

auto fileIDToPath(MemgraphConnection const & mgConnection){
    std::unordered_map<FileReference, std::string, FileReferenceHash, FileReferenceEqual> fileIDToPathMap;
    if(not mgConnection.execute(
        CipherQuery("MATCH (f:File)")
        .endl()
        .ret("ID(f)","f.path")
    )){
        throw std::runtime_error("Could not load files from database");
    }
    while(auto currentRow = mgConnection->FetchOne()){
        auto file = FileReference(currentRow->at(0).ValueInt());
        auto path = currentRow->at(1).ValueString();
        fileIDToPathMap.try_emplace(file,std::string(path));
    }
    return fileIDToPathMap;
}

auto distributeComponentsWorkload(ComponentMap const& componentMap, MemgraphClient const & mgClient) {
    auto [componentToFilesMap,fileToComponentsMap] = componentIDToFileID(mgClient.getMemgraphConnection(),componentMap);
    auto fileToPathMap = fileIDToPath(mgClient.getMemgraphConnection());
    auto multiFileComponents = std::views::filter(componentToFilesMap, [&](auto && pair) {
        return pair.second.size() > 1 && componentMap.contains(pair.first);
    });
    std::vector<ClusterWorkload> workloads;

    // Insert single file component workloads, one workload processes all single-file-components of each file
    for(auto && [file,components]: fileToComponentsMap){
        auto singleFileComponents = std::views::filter(components, [&](auto && component) {
            return componentToFilesMap.at(component).size() == 1 && componentMap.contains(component);
        });
        size_t nodeCount = 0;
        std::vector<size_t> componentIdsOfFile;
        for(auto && componentRef: singleFileComponents){
            nodeCount += componentMap.at(componentRef).size();
            componentIdsOfFile.push_back(componentRef.componentId);
        }
        std::vector<std::string> filePath = {fileToPathMap.at(file)};
        auto workload = ClusterWorkload{
            .nodes = nodeCount,
            .files = filePath,
            .components = componentIdsOfFile
        };
        workloads.push_back(workload);
    }
    // insert multi file component workloads
    for(const auto & [componentRef, files]: multiFileComponents){
        auto workload = ClusterWorkload{
            .nodes = componentMap.at(componentRef).size(),
            .files = {},
            .components = {asNodeIdType(componentRef.componentId)}
        };
        std::ranges::for_each(files, [&](auto && file) {
            workload.files.push_back(fileToPathMap.at(file));
        });
        workloads.push_back(workload);
    }
    
    return workloads;
}

int main(int argc, char *argv[]){
    CLI::App app{"AfricapolisGraphComponents"};
    std::string configFilename = "/home/lolo/Documents/fishnet/cwl/africapolis/africapolis-config.json";
    std::string componentJsonFilename = "components";
    app.add_option("-c,--config", configFilename, "Path to configuration file for graph components stage of Africapolis workflow")
        ->required()
        ->check(CLI::ExistingFile);
    app.add_option("-o,--outputComponentFileBasename", componentJsonFilename, "Basename for component output file storing the components in json format")->required();
    // CLI11_PARSE(app, argc, argv);
    MemgraphTaskConfig config{json::parse(std::ifstream(configFilename))};
    MemgraphClient mgClient = MemgraphClient(MemgraphConnection::create(config.params).value_or_throw());
    auto graph = loadIDGraph(mgClient);
    auto components = findAndStoreComponents(graph, mgClient);
    std::vector<ClusterWorkload> workloads = distributeComponentsWorkload(components, mgClient);
    nlohmann::json jsonComponents = workloads;
    std::ofstream outputFile(componentJsonFilename+".json");
    outputFile << jsonComponents.dump(4) << std::endl;
    outputFile.close();
    return 0;
}