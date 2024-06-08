#pragma once
#include <magic_enum.hpp>
#include "TaskConfig.hpp"
#include "Job.hpp"
#include "fishnet/FunctionalConcepts.hpp"
#include "NeighbouringFilesPredicates.hpp"
#include "CwlToilExecutor.hpp"
#include "CwlToolExecutor.hpp"
#include "Scheduler.hpp"

class SettlementDelineationConfig:public MemgraphTaskConfig{
private:
    constexpr static const char * JOB_DIRECTORY_KEY = "job-directory";
    constexpr static const char * CFG_DIRECTORY_KEY = "config-directory";
    constexpr static const char * LAST_JOB_TYPE_KEY = "last-job-type";
    constexpr static const char * EXECUTOR_KEY = "executor";
public:
    std::filesystem::path jobDirectory;
    std::filesystem::path workingDirectory;
    std::filesystem::path cfgDirectory;
    JobType lastJobType;
    fishnet::util::BiPredicate_t<std::filesystem::path> neighbouringFilesPredicate = NeighbouringFileTilesPredicate();
    ExecutorType executorType;
    json executorDesc;

    SettlementDelineationConfig(const json & config):MemgraphTaskConfig(config){
        const auto & cfg = this->jsonDescription;
        cfg.at(JOB_DIRECTORY_KEY).get_to(this->jobDirectory);
        cfg.at(CFG_DIRECTORY_KEY).get_to(this->cfgDirectory);
        std::string jobTypeName;
        cfg.at(LAST_JOB_TYPE_KEY).get_to(jobTypeName);
        auto asJobType = magic_enum::enum_cast<JobType>(jobTypeName);
        if(not asJobType)
            lastJobType = JobType::ANALYSIS;
        else 
            lastJobType = asJobType.value();
        std::string executorTypename;
        cfg.at(EXECUTOR_KEY).at("name").get_to(executorTypename);
        auto executorOpt = magic_enum::enum_cast<ExecutorType>(executorTypename);
        if(not executorOpt)
            throw std::runtime_error("Could not parse executor type from json\n"+cfg.dump());
        this->executorType = executorOpt.value();
        executorDesc = cfg.at(EXECUTOR_KEY);
    }

    Scheduler getSchedulerWithExecutorType (JobDAG_t && dag) {
        switch(executorType){
            case ExecutorType::CWLTOIL: {
                std::string cwlDirectory;
                executorDesc.at("cwl-directory").get_to(cwlDirectory);
                std::string flags;
                executorDesc.at("flags").get_to(flags);
                return Scheduler(std::move(dag),CwlToilExecutor(std::move(cwlDirectory),std::move(flags)),lastJobType);
            }
            case ExecutorType::CWLTOOL:{
                std::string cwlDir;
                executorDesc.at("cwl-directory").get_to(cwlDir);
                return Scheduler(std::move(dag),CwlToolExecutor(std::move(cwlDir)),lastJobType);
            }
            default:
                throw std::runtime_error("Could not create scheduler from executor type.\n"+this->jsonDescription.dump());
        }
    }
};