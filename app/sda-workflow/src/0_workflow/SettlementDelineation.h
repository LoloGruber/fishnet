#pragma once
#include <vector>
#include <filesystem>
#include <fishnet/GISFactory.hpp>
#include "Task.hpp"
#include "JobDAG.hpp"

#include "JobGenerator.hpp"
#include "SchedulerConfig.hpp"
#include "ConnectedComponentsConfig.hpp"
#include "SettlementDelineationConfig.hpp"
#include "MemgraphClient.hpp"

class SettlementDelineation: public Task{
private:
    SettlementDelineationConfig config;
    std::vector<std::filesystem::path> inputFiles;
    std::filesystem::path userConfig;
    std::filesystem::path outputPath;
    std::optional<std::filesystem::path> listenerFile;
    fishnet::util::TemporaryDirectory workingDirectory;
    std::filesystem::path jobDirectory;

public:
    SettlementDelineation(SettlementDelineationConfig && config,const std::filesystem::path & inputPath,std::filesystem::path outputPath, std::filesystem::path configPath,  std::optional<std::filesystem::path> listenerFile = std::nullopt )
    :config(std::move(config)),userConfig(std::move(configPath)),outputPath(std::move(outputPath)),listenerFile(std::move(listenerFile)){
        // Get input file(s) from path
        this->inputFiles = fishnet::GISFactory::getGISFiles(inputPath);
        // set current path to working directory / tmp directory
        std::filesystem::current_path(workingDirectory); 
        // Create job directory
        this->jobDirectory = workingDirectory.get() / std::filesystem::path("jobs/");
        this->desc["type"]="Settlement Delineation Workload Generator & Scheduler";
        this->desc["config"] = this->config.components.jsonDescription;
        this->desc["working-directory"]=this->workingDirectory.get().string();
        std::vector<std::string> inputStrings;
        std::ranges::for_each(this->inputFiles,[this,&inputStrings](const auto & file){inputStrings.push_back(file.string());});
        this->desc["inputs"]=inputStrings;
        this->desc["output"]=this->outputPath.string();
    }

    

    void mergeOutput(Scheduler & scheduler){
        size_t maxID = std::ranges::max(scheduler.getDAG().getNodes(),{},&Job::id).id;
        MergeJob mergeJob;
        MergeJob mergeEdgesJob;
        mergeJob.id=maxID+1;
        mergeEdgesJob.id  = maxID+2;
        mergeJob.file = jobDirectory / "Merge.json";
        mergeEdgesJob.file = jobDirectory / "Merge_edges.json";
        mergeJob.state = JobState::RUNNABLE;
        mergeEdgesJob.state = JobState::RUNNABLE;
        mergeJob.output = outputPath;
        mergeEdgesJob.output = fishnet::util::PathHelper::appendToFilename(outputPath,"_edges");
        for(auto && path : fishnet::GISFactory::getGISFiles(workingDirectory)){
            if(path.stem().string().starts_with(config.components.analysisOutputStem) && path.string().ends_with(".shp")){
                if(path.string().ends_with("_edges.shp"))
                    mergeEdgesJob.inputs.push_back(std::move(path));
                else
                    mergeJob.inputs.push_back(std::move(path));
            }
        }
        if(mergeJob.inputs.empty())
            return;
        if(mergeJob.inputs.size() == 1){
            fishnet::Shapefile analysisOutput {mergeJob.inputs.front()};
            fishnet::Shapefile output {mergeJob.output};
            output.remove();
            analysisOutput.copy(output.getPath());
            if(mergeEdgesJob.inputs.size() == 1){
                fishnet::Shapefile analysisEdgeOutput {mergeEdgesJob.inputs.front()};
                fishnet::Shapefile edgeOutput {mergeEdgesJob.output};
                edgeOutput.remove();
                analysisEdgeOutput.copy(edgeOutput.getPath());
            }
            return;
        }
        JobWriter::write(mergeJob);
        scheduler.getDAG().addNode(mergeJob);
        if(not mergeEdgesJob.inputs.empty()){
            JobWriter::write(mergeEdgesJob);
            scheduler.getDAG().addNode(mergeEdgesJob);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        scheduler.schedule();
    }

    void writeConfig(){
        json jsonCfg = json::parse(std::ifstream(userConfig));
        jsonCfg["working-directory"] = this->workingDirectory.get();
        std::ofstream cfgStream {fishnet::util::PathHelper::appendToFilename(outputPath,"_Config.json")};
        cfgStream << jsonCfg.dump(4) << std::endl;
    }

    void cleanup(JobDAG_t & jobDag){
        std::filesystem::remove_all(jobDirectory);
        std::filesystem::remove(jobDirectory);
        workingDirectory.clear();
        jobDag.getAdjacencyContainer().clearAll();
        MemgraphClient(MemgraphConnection(jobDag.getAdjacencyContainer().getConnection())).clearAll();
        if(MemgraphConnection::hasSession())
            CipherQuery().match("(s:Session{id:$sid})").setInt("sid",MemgraphConnection::getSession().id()).del("s").executeAndDiscard(MemgraphConnection(jobDag.getAdjacencyContainer().getConnection()));
    }

    SchedulerObserver_t DAGListener() const noexcept {
        assert(listenerFile.has_value());
        return [this](const Scheduler & scheduler){
            json output;
            auto nodes = scheduler.getDAG().getNodes();
            output["ALL"] = fishnet::util::size(nodes);
            for(JobState state: {JobState::RUNNABLE, JobState::RUNNING,JobState::SUCCEED, JobState::FAILED, JobState::ABORTED}) {
                output[magic_enum::enum_name(state)] = std::ranges::count_if(nodes,[&state]( const Job & job){return job.state == state;});
            }
            std::ofstream os {listenerFile.value()};
            os << std::setw(4) << output << std::endl;
        };
    }

    void run() override {
        if(inputFiles.empty())
            throw std::runtime_error("No input files provided");
        if(not std::filesystem::is_empty(workingDirectory))
            throw std::runtime_error("Working directory is not empty");
        // add working directory to config
        auto workflowConfigPath = workingDirectory.get() / std::filesystem::path("cfg.json");
        json updateCfg = this->config.scheduler.jsonDescription;
        updateCfg[TaskConfig::WORKING_DIRECTORY_KEY] = workingDirectory.get();
        std::ofstream os {workflowConfigPath};
        os << updateCfg.dump(4) << std::endl;
        MemgraphConnection connection = MemgraphConnection::create(config.scheduler.params).value_or_throw();
        Session workflowSession;
        if(config.concurrentRuns){
            workflowSession = Session::makeUnique(connection); // create label-isolated session, save id for later stages
            MemgraphConnection::setSession(workflowSession);
        }
        Scheduler scheduler = config.scheduler.getSchedulerWithExecutorType(loadDAG(JobAdjacency(std::move(connection))));
        if(listenerFile){
            scheduler.addListener(DAGListener());
        }
        auto & jobDag = scheduler.getDAG();
        JobGenerator jobGenerator {JobGeneratorConfig(config.jobGenerator),workingDirectory,workflowConfigPath};
        fishnet::util::AutomaticTemporaryDirectory tmp;
        if(config.jobGenerator.splits > 0){
            jobGenerator.generateWSFSplitJobs(inputFiles,tmp,jobDag);
            scheduler.schedule();
            jobGenerator.generate(fishnet::GISFactory::getGISFiles(tmp),jobDag);
        }else {
            jobGenerator.generate(inputFiles,jobDag);
        }
        scheduler.schedule();
        if(config.scheduler.lastJobType >= JobType::MERGE){
            mergeOutput(scheduler);
        }
        writeConfig();
        if(config.cleanup && scheduler.failedJobs().empty())
            cleanup(scheduler.getDAG());
    }
};