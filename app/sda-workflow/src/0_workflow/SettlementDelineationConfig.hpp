#pragma once
#include <fishnet/FunctionalConcepts.hpp>
#include <magic_enum.hpp>
#include "TaskConfig.hpp"
#include "Job.hpp"
#include "JobGeneratorConfig.hpp"
#include "SchedulerConfig.hpp"

class SettlementDelineationConfig{
private:
    constexpr static const char * CLEANUP_KEY = "cleanup";    
    constexpr static const char * CONCURRENT_RUNS_KEY = "concurrent-runs";
public:
    JobGeneratorConfig jobGenerator;
    SchedulerConfig scheduler;
    bool cleanup;
    bool concurrentRuns;

    SettlementDelineationConfig(const json & config):jobGenerator(config),scheduler(config){
        config.at(CLEANUP_KEY).get_to(this->cleanup);
        config.at(CONCURRENT_RUNS_KEY).get_to(this->concurrentRuns);
    }
};