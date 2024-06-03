#pragma once
#include <magic_enum.hpp>
#include "TaskConfig.hpp"
#include "Job.hpp"
#include "fishnet/FunctionalConcepts.hpp"
#include "NeighbouringFilesPredicates.hpp"

struct SettlementDelineationConfig:public MemgraphTaskConfig{
    constexpr static const char * WORKING_DIRECTORY_KEY = "working-directory";
    constexpr static const char * JOB_DIRECTORY_KEY = "job-directory";
    constexpr static const char * CFG_DIRECTORY_KEY = "config-directory";
    constexpr static const char * LAST_JOB_TYPE_KEY = "last-job-type";

    std::filesystem::path jobDirectory;
    std::filesystem::path workingDirectory;
    std::filesystem::path cfgDirectory;
    JobType lastJobType;
    fishnet::util::BiPredicate_t<std::filesystem::path> neighbouringFilesPredicate = NeighbouringFileTilesPredicate();

    SettlementDelineationConfig(const json & config):MemgraphTaskConfig(config){
        const auto & cfg = this->jsonDescription;
        cfg.at(WORKING_DIRECTORY_KEY).get_to(this->workingDirectory);
        cfg.at(JOB_DIRECTORY_KEY).get_to(this->jobDirectory);
        cfg.at(CFG_DIRECTORY_KEY).get_to(this->cfgDirectory);
        std::string jobTypeName;
        cfg.at(LAST_JOB_TYPE_KEY).get_to(jobTypeName);
        auto asJobType = magic_enum::enum_cast<JobType>(jobTypeName);
        if(not asJobType)
            lastJobType = JobType::ANALYSIS;
        else 
            lastJobType = asJobType.value();
    }
};