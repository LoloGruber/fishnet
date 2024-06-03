#include <fishnet/GraphFactory.hpp>
#include <fishnet/BFSAlgorithm.hpp>
#include "MemgraphClient.hpp"
#include "Task.hpp"
#include "ConnectedComponentsConfig.hpp"
#include "JobWriter.hpp"
#include "JobDAG.hpp"

class ConnectedComponentsTask: public Task{
private:
    ConnectedComponentsConfig config;
    std::filesystem::path jobDirectory;
    std::filesystem::path cfgDirectory;
    size_t nextJobID;

    struct ComponentFileJob{
        std::vector<std::string> files;
        std::vector<uint64_t> components;
    };

public:
    ConnectedComponentsTask(ConnectedComponentsConfig && config,std::filesystem::path && jobDirectory,std::filesystem::path && cfgDirectory,size_t nextJobID):config(std::move(config)),jobDirectory(std::move(jobDirectory)),cfgDirectory(std::move(cfgDirectory)),nextJobID(nextJobID){
        this->writeDescLine("ComponentTask")
        .writeDescLine("-Config:")
        .indentDescLine(this->config.jsonDescription.dump())
        .writeDescLine("-Job Directory:")
        .indentDescLine(this->jobDirectory.string())
        .writeDescLine("-Cfg Directory:")
        .indentDescLine(this->cfgDirectory.string())
        .writeDesc("-Next Job Id: ").writeDesc(nextJobID);
    }

    std::unordered_map<uint64_t,std::vector<std::string>> queryPathsForComponent(const std::vector<ComponentReference> & componentIds, const MemgraphConnection & memgraphConnection){
        std::unordered_map<uint64_t,std::vector<std::string>> componentToFilesMap;
        std::vector<mg::Value> componentValues;
        for(auto componentRef : componentIds)
            componentValues.push_back(mg::Value(componentRef.componentId));
        if(not ParameterizedQuery()
                .line("UNWIND $data as component_id")
                .line("MATCH (c:Component)<-[:part_of]-(:Node)-[:stored]->(f:File)")
                .line("WHERE ID(c)=component_id")
                .set("data",mg::Value(mg::List(componentValues)))
                .line("RETURN DISTINCT component_id,f.path;")
                .execute(memgraphConnection)
        ) throw std::runtime_error("Could not execute query to find files part of a component");
        while(auto currentRow = memgraphConnection->FetchOne()){
            auto id = currentRow->at(0).ValueInt();
            auto file = currentRow->at(1).ValueString();
            if(not componentToFilesMap.contains(id))
                componentToFilesMap.try_emplace(id,std::vector<std::string>());
            componentToFilesMap.at(id).push_back(std::string(file)); 
        }
        return componentToFilesMap;
    }



    void run() override {
       auto ExpMemgraphConnection = MemgraphConnection::create(config.params).transform([](auto && connection){return MemgraphClient(std::move(connection));});
        const auto & memgraphClient = getExpectedOrThrowError(ExpMemgraphConnection);
        auto nodesList = memgraphClient.nodes();
        auto adjMap = memgraphClient.edges();
        auto graph = fishnet::graph::GraphFactory::UndirectedGraph<size_t>();
        graph.addNodes(nodesList);
        for(auto && [node,neigbours]:adjMap){ 
            for(auto neighbour: neigbours){ // use by value since datatype is very cheap to copy
                graph.addEdge(node,neighbour); //
            }
        }
        auto components = fishnet::graph::BFS::connectedComponents(graph).get();
        auto componentIds = memgraphClient.createComponents(components);
        std::unordered_map<uint64_t,std::vector<std::string>> componentToFilesMap = queryPathsForComponent(componentIds,memgraphClient.getMemgraphConnection());
        std::unordered_map<std::string,std::vector<uint64_t>> fileToComponentsMap;
        std::vector<ComponentFileJob> contractionJobs;
        for(auto && [component,files]:componentToFilesMap){
            if(files.size() == 1){
                const auto & file = *std::ranges::begin(files);
                fileToComponentsMap.try_emplace(file,std::vector<uint64_t>());
                fileToComponentsMap.at(file).push_back(component);
            }else {
                contractionJobs.emplace_back(std::move(files), std::vector<uint64_t>({component}));
            }
        }
        for(auto && [file,components]:fileToComponentsMap){
            contractionJobs.emplace_back(std::vector<std::string>({file}),std::move(components));
        }
        auto exp = MemgraphConnection::create(config.params).transform([](auto && conn){return JobAdjacency(std::move(conn));});
        auto && jobAdj = getExpectedOrThrowError(exp);
        auto jobDAG = loadDAG(std::move(jobAdj));
        auto stringsToPathsMapper = [](std::vector<std::string> const & files){
            std::vector<std::filesystem::path> paths;
            paths.reserve(files.size());
            std::ranges::for_each(files,[&paths](const auto & file){paths.emplace_back(file);});
            return paths;
        };
        for(auto && [files,components]: contractionJobs){
            ContractionJob contractionJob;
            contractionJob.id = nextJobID++;
            auto jobFilename = std::filesystem::path(files[0]).filename().replace_extension(".json");
            contractionJob.file = jobDirectory / std::filesystem::path {"Contraction_"+jobFilename.string()};
            contractionJob.config = cfgDirectory / std::filesystem::path{"contraction.json"};
            contractionJob.inputs = stringsToPathsMapper(files);
            contractionJob.components = components;
            contractionJob.state = JobState::RUNNABLE;
            contractionJob.type = JobType::CONTRACTION;
            auto outputName = config.contractionOutputStem + "_" +std::to_string(contractionJob.id);
            contractionJob.outputStem = outputName;
            JobWriter::write(contractionJob);
            AnalysisJob analysisJob;
            analysisJob.id = nextJobID++;
            analysisJob.file = jobDirectory / std::filesystem::path {"Analysis_"+jobFilename.string()};
            analysisJob.config = cfgDirectory / std::filesystem::path{"analysis.json"};
            analysisJob.state = JobState::RUNNABLE;
            analysisJob.type = JobType::ANALYSIS;
            analysisJob.outputStem = config.analysisOutputStem;
            analysisJob.input = fishnet::Shapefile(files[0]).changeFilename(outputName).getPath();
            JobWriter::write(analysisJob);
            jobDAG.addEdge(contractionJob,analysisJob);
        }
    }
};