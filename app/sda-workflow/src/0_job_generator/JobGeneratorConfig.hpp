#pragma once
#include <magic_enum.hpp>
#include "TaskConfig.hpp"
#include "Job.hpp"
#include "fishnet/FunctionalConcepts.hpp"
#include "NeighbouringFilesPredicates.hpp"

class JobGeneratorConfig:public MemgraphTaskConfig{
private:
    constexpr static const char * LAST_JOB_TYPE_KEY = "last-job-type";
    constexpr static const char * NEIGHBOURING_FILES_PREDICATE_KEY = "neighbouring-files-predicate";
    constexpr static const char * SPLIT_KEY ="splits";
public:
    JobType lastJobType;
    u_int32_t splits = 0;
    fishnet::util::BiPredicate_t<std::filesystem::path> neighbouringFilesPredicate;
    NeighbouringFilesPredicateType neighbouringFilesPredicateType;

    JobGeneratorConfig(const json & config):MemgraphTaskConfig(config){
        const auto & cfg = this->jsonDescription;
        std::string jobTypeName;
        cfg.at(LAST_JOB_TYPE_KEY).get_to(jobTypeName);
        auto asJobType = magic_enum::enum_cast<JobType>(jobTypeName);
        if(not asJobType)
            lastJobType = JobType::ANALYSIS;
        else 
            lastJobType = asJobType.value();
        std::string neighbouringFilesPredicateTypeName;
        cfg.at(NEIGHBOURING_FILES_PREDICATE_KEY).get_to(neighbouringFilesPredicateTypeName);
        auto neighbouringPredicateType = magic_enum::enum_cast<NeighbouringFilesPredicateType>(neighbouringFilesPredicateTypeName);
        auto neighbouringPredicate = neighbouringPredicateType.and_then([desc = cfg.at(NEIGHBOURING_FILES_PREDICATE_KEY)](NeighbouringFilesPredicateType type){
            return fromJson(type,desc);
        });
        if(not neighbouringPredicate)
            throw std::runtime_error("Could not parse neighbouring files predicate type");
        neighbouringFilesPredicateType = neighbouringPredicateType.value();
        neighbouringFilesPredicate = neighbouringPredicate.value();
        if(cfg.contains(SPLIT_KEY)){
            cfg.at(SPLIT_KEY).get_to(this->splits);
        }
    }
};