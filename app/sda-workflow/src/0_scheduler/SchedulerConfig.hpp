#pragma once
#include <magic_enum.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include "TaskConfig.hpp"
#include "Job.hpp"
#include "CwlToilExecutor.hpp"
#include "CwlToolExecutor.hpp"
#include "Scheduler.hpp"

class SchedulerConfig:public MemgraphTaskConfig{
private:
    constexpr static const char * LAST_JOB_TYPE_KEY = "last-job-type";
    constexpr static const char * EXECUTOR_KEY = "executor";
    constexpr static const char * CONCURRENCY_KEY="hardware-concurrency";
public:
    JobType lastJobType;
    ExecutorType executorType;
    json executorDesc;
    size_t concurrency;


    SchedulerConfig(const json & config):MemgraphTaskConfig(config){
        const auto & cfg = this->jsonDescription;
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
        if(cfg.contains(CONCURRENCY_KEY))
            cfg.at(CONCURRENCY_KEY).get_to(concurrency);
        else    
            concurrency = std::thread::hardware_concurrency();
    }

    Scheduler getSchedulerWithExecutorType (JobDAG_t && dag) {
        switch(executorType){
            case ExecutorType::CWLTOIL: {
                std::string cwlDirectory;
                executorDesc.at("cwl-directory").get_to(cwlDirectory);
                std::string flags;
                executorDesc.at("flags").get_to(flags);
                return Scheduler(std::move(dag),CwlToilExecutor(std::move(cwlDirectory),std::move(flags)),lastJobType,concurrency);
            }
            case ExecutorType::CWLTOOL:{
                std::string cwlDir;
                executorDesc.at("cwl-directory").get_to(cwlDir);
                std::string flags;
                executorDesc.at("flags").get_to(flags);
                return Scheduler(std::move(dag),CwlToolExecutor(std::move(cwlDir),std::move(flags)),lastJobType,concurrency);
            }
            default:
                throw std::runtime_error("Could not create scheduler from executor type.\n"+this->jsonDescription.dump());
        }
    }
};