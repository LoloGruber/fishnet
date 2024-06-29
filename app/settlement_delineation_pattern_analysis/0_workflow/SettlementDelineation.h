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
    std::filesystem::path workingDirectory;
    std::filesystem::path outputPath;

public:
    SettlementDelineation(const json & cfg, std::vector<std::filesystem::path> && inputFiles,std::filesystem::path outputPath, std::filesystem::path workingDirectory = std::filesystem::current_path())
    :schedulerConfig(cfg),jobGeneratorConfig(cfg),connectedComponentsConfig(cfg),inputFiles(std::move(inputFiles)),workingDirectory(std::move(workingDirectory)),outputPath(std::move(outputPath)){
        this->desc["type"]="Settlement Delineation Workload Generator & Scheduler";
        this->desc["config"] = cfg;
        this->desc["working-directory"]=this->workingDirectory.string();
        std::vector<std::string> inputStrings;
        std::ranges::for_each(this->inputFiles,[this,&inputStrings](const auto & file){inputStrings.push_back(file.string());});
        this->desc["inputs"]=inputStrings;
        this->desc["output"]=this->outputPath.string();
    }

    void run() override {
        if(inputFiles.empty())
            throw std::runtime_error("No input files provided");
        auto exp = MemgraphConnection::create(schedulerConfig.params).transform([](auto && conn){return JobAdjacency(std::move(conn));});
        auto && jobAdj = getExpectedOrThrowError(exp);
        Scheduler scheduler = schedulerConfig.getSchedulerWithExecutorType(loadDAG(std::move(jobAdj)));
        auto & jobDag = scheduler.getDAG();
        JobGeneratorConfig copy = jobGeneratorConfig;
        JobGenerator jobGenerator {std::move(copy),workingDirectory};
        auto tmpDir = fishnet::util::TemporaryDirectory(workingDirectory / std::filesystem::path("tmp/") );
        if(jobGeneratorConfig.cleanup)
            jobGenerator.cleanup(jobDag);
        if(jobGeneratorConfig.splits > 0){
            jobGenerator.generateWSFSplitJobs(inputFiles,tmpDir.getDirectory(),jobDag);
            scheduler.schedule();
            jobGenerator.generate(fishnet::GISFactory::getGISFiles(tmpDir.getDirectory()),jobDag);
        }else {
            jobGenerator.generate(inputFiles,jobDag);
        }
        scheduler.schedule();
        if(schedulerConfig.lastJobType >= JobType::MERGE){
            MergeJob mergeJob;
            mergeJob.id=(size_t)-1;
            mergeJob.file = jobGeneratorConfig.jobDirectory / "Merge.json";
            mergeJob.state = JobState::RUNNABLE;
            for(auto && path : fishnet::GISFactory::getGISFiles(workingDirectory)){
                if(path.stem().string().starts_with(connectedComponentsConfig.analysisOutputStem) && path.string().ends_with(".shp")){
                    mergeJob.inputs.push_back(std::move(path));
                }
            }
            mergeJob.output = outputPath;
            JobWriter::write(mergeJob);
            scheduler.getDAG().addNode(mergeJob);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            scheduler.schedule();
        }
    }
};