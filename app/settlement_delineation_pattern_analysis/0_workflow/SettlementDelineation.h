#pragma once
#include <vector>
#include <filesystem>

#include "Task.hpp"
#include "JobDAG.hpp"

#include "JobGenerator.hpp"
#include "SchedulerConfig.hpp"

class SettlementDelineation: public Task{
private:
    SchedulerConfig schedulerConfig;
    JobGeneratorConfig jobGeneratorConfig;
    std::vector<std::filesystem::path> inputFiles;
    std::filesystem::path workingDirectory;

public:
    SettlementDelineation(const json & cfg, std::vector<std::filesystem::path> && inputFiles,std::filesystem::path workingDirectory = std::filesystem::current_path())
    :schedulerConfig(cfg),jobGeneratorConfig(cfg),inputFiles(std::move(inputFiles)),workingDirectory(std::move(workingDirectory)){
        this->writeDescLine("Settlement Delineation Workload Generator & Scheduler:")
        .writeDescLine("Config:")
        .writeDescLine(cfg.dump(4))
        .writeDescLine("Working Directory:")
        .indentDescLine(this->workingDirectory.string())
        .writeDescLine("Inputs:");
        std::ranges::for_each(this->inputFiles,[this](const auto & file){this->indentDescLine(file.string());});
    }

    void run() override {
        if(inputFiles.empty())
            throw std::runtime_error("No input files provided");
        auto exp = MemgraphConnection::create(schedulerConfig.params).transform([](auto && conn){return JobAdjacency(std::move(conn));});
        auto && jobAdj = getExpectedOrThrowError(exp);
        auto jobDAG = loadDAG(std::move(jobAdj));
        JobGenerator jobGenerator {std::move(jobGeneratorConfig),workingDirectory};
        jobGenerator.generate(inputFiles,jobDAG);
        Scheduler scheduler = schedulerConfig.getSchedulerWithExecutorType(std::move(jobDAG));
        scheduler.schedule();
    }
};