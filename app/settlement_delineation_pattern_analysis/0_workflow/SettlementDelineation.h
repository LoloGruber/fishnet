#pragma once
#include <vector>
#include <filesystem>

#include "Task.hpp"
#include "JobDAG.hpp"

#include "JobGenerator.hpp"
#include "SettlementDelineationConfig.hpp"
#include "Scheduler.hpp"

class SettlementDelineation: public Task{
private:
    SettlementDelineationConfig config;
    std::vector<std::filesystem::path> inputFiles;
    std::filesystem::path workingDirectory;

public:
    SettlementDelineation(SettlementDelineationConfig && config,  std::vector<std::filesystem::path> && inputFiles)
    :config(std::move(config)),inputFiles(std::move(inputFiles)),workingDirectory(std::filesystem::current_path()){
        this->writeDescLine("Settlement Delineation Workload Generator & Scheduler:")
        .writeDescLine("Config:")
        .writeDescLine(this->config.jsonDescription.dump(4))
        .writeDescLine("Working Directory:")
        .indentDescLine(this->workingDirectory.string())
        .writeDescLine("Inputs:");
        std::ranges::for_each(this->inputFiles,[this](const auto & file){this->indentDescLine(file.string());});
    }

    void run() override {
        if(inputFiles.empty())
            throw std::runtime_error("No input files provided");
        JobGenerator jobGenerator {config.neighbouringFilesPredicate,config.jobDirectory,config.cfgDirectory,workingDirectory,config.lastJobType};
        auto exp = MemgraphConnection::create(config.params).transform([](auto && conn){return JobAdjacency(std::move(conn));});
        auto && jobAdj = getExpectedOrThrowError(exp);
        auto jobDAG = loadDAG(std::move(jobAdj));
        jobDAG.clear();
        jobGenerator.generate(inputFiles,jobDAG);
        Scheduler scheduler = config.getSchedulerWithExecutorType(std::move(jobDAG));
        scheduler.schedule();
    }
};