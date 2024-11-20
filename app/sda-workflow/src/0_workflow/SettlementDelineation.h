#pragma once
#include <vector>
#include <filesystem>
#include <fishnet/GISFactory.hpp>
#include "Task.hpp"
#include "JobDAG.hpp"

#include "JobGenerator.hpp"
#include "SchedulerConfig.hpp"
#include "ConnectedComponentsConfig.hpp"

class SettlementDelineation: public Task{
private:
    SchedulerConfig schedulerConfig;
    JobGeneratorConfig jobGeneratorConfig;
    ConnectedComponentsConfig connectedComponentsConfig;
    std::vector<std::filesystem::path> inputFiles;
    std::filesystem::path cfgFile;
    std::filesystem::path outputPath;
    fishnet::util::TemporaryDirectory workingDirectory;

public:
    SettlementDelineation(const json & cfg,const std::filesystem::path & inputPath,std::filesystem::path outputPath, std::filesystem::path configPath)
    :schedulerConfig(cfg),jobGeneratorConfig(cfg),connectedComponentsConfig(cfg),cfgFile(std::move(configPath)),outputPath(std::move(outputPath)){
        this->inputFiles = fishnet::GISFactory::getGISFiles(inputPath);
        std::filesystem::current_path(workingDirectory); // set current path to working directory
        this->desc["type"]="Settlement Delineation Workload Generator & Scheduler";
        this->desc["config"] = cfg;
        this->desc["working-directory"]=this->workingDirectory.get().string();
        std::vector<std::string> inputStrings;
        std::ranges::for_each(this->inputFiles,[this,&inputStrings](const auto & file){inputStrings.push_back(file.string());});
        this->desc["inputs"]=inputStrings;
        this->desc["output"]=this->outputPath.string();
    }

    void run() override {
        if(inputFiles.empty())
            throw std::runtime_error("No input files provided");
        if(not std::filesystem::is_empty(workingDirectory))
            throw std::runtime_error("Working directory is not empty");
        Scheduler scheduler = schedulerConfig.getSchedulerWithExecutorType(loadDAG(JobAdjacency(MemgraphConnection::create(schedulerConfig.params).value_or_throw())));
        auto & jobDag = scheduler.getDAG();
        JobGenerator jobGenerator {JobGeneratorConfig(jobGeneratorConfig),workingDirectory,cfgFile};
        fishnet::util::AutomaticTemporaryDirectory tmp;
        if(jobGeneratorConfig.cleanup)
            jobGenerator.cleanup(jobDag);
        if(jobGeneratorConfig.splits > 0){
            jobGenerator.generateWSFSplitJobs(inputFiles,tmp,jobDag);
            scheduler.schedule();
            jobGenerator.generate(fishnet::GISFactory::getGISFiles(tmp),jobDag);
        }else {
            jobGenerator.generate(inputFiles,jobDag);
        }
        scheduler.schedule();
        if(schedulerConfig.lastJobType >= JobType::MERGE){
            MergeJob mergeJob;
            MergeJob mergeEdgesJob;
            mergeJob.id=1000000000000;
            mergeEdgesJob.id  = 99999999999;
            mergeJob.file = jobGeneratorConfig.jobDirectory / "Merge.json";
            mergeEdgesJob.file = jobGeneratorConfig.jobDirectory / "Merge_edges.json";
            mergeJob.state = JobState::RUNNABLE;
            mergeEdgesJob.state = JobState::RUNNABLE;
            for(auto && path : fishnet::GISFactory::getGISFiles(workingDirectory)){
                if(path.stem().string().starts_with(connectedComponentsConfig.analysisOutputStem) && path.string().ends_with(".shp")){
                    if(path.string().ends_with("_edges.shp"))
                        mergeEdgesJob.inputs.push_back(std::move(path));
                    else
                        mergeJob.inputs.push_back(std::move(path));
                }
            }
            mergeJob.output = outputPath;
            mergeEdgesJob.output = fishnet::util::PathHelper::appendToFilename(outputPath,"_edges");
            JobWriter::write(mergeJob);
            scheduler.getDAG().addNode(mergeJob);
            if(not mergeEdgesJob.inputs.empty()){
                JobWriter::write(mergeEdgesJob);
                scheduler.getDAG().addNode(mergeEdgesJob);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            scheduler.schedule();
        }
        //TODO clear workingdirectory if cfg enables and no failed jobs
        json cfg = json::parse(std::ifstream(cfgFile));
        std::ofstream cfgStream {fishnet::util::PathHelper::appendToFilename(outputPath,"_cfg.json")};
        cfgStream << cfg.dump(4) << std::endl;
    }
};