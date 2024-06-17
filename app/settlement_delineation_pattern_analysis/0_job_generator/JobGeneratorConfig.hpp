#pragma once
#include <magic_enum.hpp>
#include "TaskConfig.hpp"
#include "Job.hpp"
#include "fishnet/FunctionalConcepts.hpp"
#include "NeighbouringFilesPredicates.hpp"

class JobGeneratorConfig:public MemgraphTaskConfig{
private:
    constexpr static const char * JOB_DIRECTORY_KEY = "job-directory";
    constexpr static const char * CFG_DIRECTORY_KEY = "config-directory";
    constexpr static const char * LAST_JOB_TYPE_KEY = "last-job-type";
    constexpr static const char * NEIGHBOURING_FILES_PREDICATE_KEY = "neighbouring-files-predicate";
    constexpr static const char * CLEANUP_FIRST_KEY = "cleanup-first";
public:
    std::filesystem::path jobDirectory;
    std::filesystem::path cfgDirectory;
    JobType lastJobType;
    bool cleanup;
    fishnet::util::BiPredicate_t<std::filesystem::path> neighbouringFilesPredicate;

    JobGeneratorConfig(const json & config):MemgraphTaskConfig(config){
        const auto & cfg = this->jsonDescription;
        cfg.at(JOB_DIRECTORY_KEY).get_to(this->jobDirectory);
        cfg.at(CFG_DIRECTORY_KEY).get_to(this->cfgDirectory);
        cfg.at(CLEANUP_FIRST_KEY).get_to(this->cleanup);
        std::string jobTypeName;
        cfg.at(LAST_JOB_TYPE_KEY).get_to(jobTypeName);
        auto asJobType = magic_enum::enum_cast<JobType>(jobTypeName);
        if(not asJobType)
            lastJobType = JobType::ANALYSIS;
        else 
            lastJobType = asJobType.value();
        std::string neighbouringFilesPredicateTypeName;
        cfg.at(NEIGHBOURING_FILES_PREDICATE_KEY).get_to(neighbouringFilesPredicateTypeName);
        auto neighbouringPredicate = magic_enum::enum_cast<NeighbouringFilesPredicateType>(neighbouringFilesPredicateTypeName).and_then([desc = cfg.at(NEIGHBOURING_FILES_PREDICATE_KEY)](NeighbouringFilesPredicateType type){
            return fromJson(type,desc);
        });
        if(not neighbouringPredicate)
            throw std::runtime_error("Could not parse neighbouring files predicate type");
        neighbouringFilesPredicate = neighbouringPredicate.value();
    }
};