#pragma once
#include <filesystem>

enum class JobType {
    FILTER,NEIGHBOURS,COMPONENTS,CONTRACTION,ANALYSIS,UNDEFINED
};

enum class JobState{
    RUNNABLE,RUNNING,SUCCEED,FAILED,UNDEFINED
};

struct Job{
    size_t id;
    std::filesystem::path file;
    JobType type;
    JobState state;

    Job()=default;
    Job(size_t id, std::filesystem::path filePath, JobType type,JobState state):id(id),file(std::move(filePath)),type(type),state(state){};
    
    bool updateStatus(JobState newStatus) noexcept {
        if(state==newStatus)
            return false;
        state = newStatus;
        return true;
    }

    bool operator==(const Job & other)const noexcept {
        return this->id == other.id;
    }
};