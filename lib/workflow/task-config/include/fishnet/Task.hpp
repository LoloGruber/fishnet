#pragma once
#include <fishnet/StopWatch.h>
#include <spdlog/spdlog.h>

/**
 * @brief Common super class for all tasks
 * Execution time is tracked, Brings logger into scope 
 * RAII type -> execution time is printed on destruction
 */
class Task {
private: 
    std::string taskName;
    fishnet::util::StopWatch stopWatch;
protected:
    size_t workflowID = 0;
public:
    Task(std::string taskName):taskName(std::move(taskName)){
        spdlog::info("Launching {}",this->taskName);
    }

    Task(std::string taskName, size_t workflowID):taskName(std::move(taskName)), workflowID(workflowID){
        spdlog::info("Launching {} with workflow id {}",this->taskName,workflowID);
    }

    constexpr static std::string FISHNET_ID_FIELD = "FISHNET_ID";

    virtual ~Task(){
        double duration =stopWatch.stop();
        spdlog::info("Completed {}", this->taskName);
        spdlog::info("Time elapsed in seconds:{}",duration);
    }
};