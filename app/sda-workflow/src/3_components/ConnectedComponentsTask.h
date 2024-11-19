#include <fishnet/GraphFactory.hpp>
#include <fishnet/BFSAlgorithm.hpp>
#include <fishnet/PathHelper.h>
#include "MemgraphClient.hpp"
#include "Task.hpp"
#include "ConnectedComponentsConfig.hpp"
#include "JobWriter.hpp"
#include "JobDAG.hpp"

class ConnectedComponentsTask: public Task{
private:
    ConnectedComponentsConfig config;
    std::filesystem::path jobDirectory;
    std::filesystem::path cfgFile;
    size_t nextJobID;

    struct ComponentFileJob{
        std::vector<std::string> files;
        std::vector<uint64_t> components;
    };

public:
    ConnectedComponentsTask(ConnectedComponentsConfig && config,std::filesystem::path && jobDirectory,std::filesystem::path && cfgFile,size_t nextJobID)
    :config(std::move(config)),jobDirectory(std::move(jobDirectory)),nextJobID(nextJobID){
        if(std::filesystem::is_symlink(cfgFile))
            cfgFile = std::filesystem::read_symlink(cfgFile);
        this->cfgFile = std::filesystem::absolute(cfgFile);
        this->desc["type"]="COMPONENTS";
        this->desc["config"]=this->config.jsonDescription;
        this->desc["job-directory"]=this->jobDirectory.string();
        this->desc["cfg-file"]=this->cfgFile.string();
        this->desc["next-job-id"]=this->nextJobID;
    }

    std::unordered_map<uint64_t,std::vector<std::string>> queryPathsForComponent(const std::vector<ComponentReference> & componentIds, const MemgraphConnection & memgraphConnection){
        std::unordered_map<uint64_t,std::vector<std::string>> componentToFilesMap;
        std::vector<mg::Value> componentValues;
        for(auto componentRef : componentIds)
            componentValues.push_back(mg::Value(componentRef.componentId));
        if(not CipherQuery("UNWIND $data as component_id").endl()
                .match(Relation{
                    .from={.name="n",.label=Label::Settlement},
                    .label=Label::part_of,
                    .to={.name="c",.label=Label::Component}
                }).where("ID(c)=component_id")
                .match(Relation{
                    .from={.name="n"},
                    .label=Label::stored,
                    .to={.name="f",.label=Label::File}
                })
                .set("data",mg::Value(mg::List(componentValues)))
                .ret("DISTINCT component_id;f.path")
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
        this->desc["Connected Components"]=components.size();
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
        
        for(auto && [files,componentIdList]: contractionJobs){
            ContractionJob contractionJob;
            contractionJob.id = nextJobID++;
            auto filenameStem = "_"+std::filesystem::path(files[0]).stem().string()+"_Component_"+std::to_string(componentIdList[0]);
            contractionJob.file = jobDirectory / std::filesystem::path("Contraction"+filenameStem).replace_extension(".json");
            contractionJob.config = cfgFile;
            contractionJob.inputs = stringsToPathsMapper(files);
            contractionJob.components = componentIdList;
            contractionJob.state = JobState::RUNNABLE;
            contractionJob.type = JobType::CONTRACTION;
            auto outputName = config.contractionOutputStem + filenameStem;
            contractionJob.outputStem = outputName;
            JobWriter::write(contractionJob);
            AnalysisJob analysisJob;
            analysisJob.id = nextJobID++;
            analysisJob.file = jobDirectory / std::filesystem::path("Analysis"+filenameStem).replace_extension(".json");
            analysisJob.config = cfgFile;
            analysisJob.state = JobState::RUNNABLE;
            analysisJob.type = JobType::ANALYSIS;
            analysisJob.input = fishnet::util::PathHelper::changeFilename(files[0],outputName);
            analysisJob.outputStem = config.analysisOutputStem + filenameStem;
            JobWriter::write(analysisJob);
            jobDAG.addEdge(contractionJob,analysisJob);
        }
    }
};