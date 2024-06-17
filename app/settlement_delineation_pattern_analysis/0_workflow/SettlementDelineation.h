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
        this->writeDescLine("Settlement Delineation Workload Generator & Scheduler:")
        .writeDescLine("Config:")
        .writeDescLine(cfg.dump(4))
        .writeDescLine("Working Directory:")
        .indentDescLine(this->workingDirectory.string())
        .writeDescLine("Inputs:");
        std::ranges::for_each(this->inputFiles,[this](const auto & file){this->indentDescLine(file.string());});
        this->writeDescLine("Output:")
        .indentDescLine(this->outputPath.string());
    }

    void run() override {
        if(inputFiles.empty())
            throw std::runtime_error("No input files provided");
        auto exp = MemgraphConnection::create(schedulerConfig.params).transform([](auto && conn){return JobAdjacency(std::move(conn));});
        auto && jobAdj = getExpectedOrThrowError(exp);
        auto jobDAG = loadDAG(std::move(jobAdj));
        JobGeneratorConfig copy = jobGeneratorConfig;
        JobGenerator jobGenerator {std::move(copy),workingDirectory};
        jobGenerator.generate(inputFiles,jobDAG);
        Scheduler scheduler = schedulerConfig.getSchedulerWithExecutorType(std::move(jobDAG));
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