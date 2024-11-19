#pragma once
#include <filesystem>

enum class JobType {
    SPLIT,FILTER,NEIGHBOURS,COMPONENTS,CONTRACTION,ANALYSIS,MERGE,UNDEFINED
};

static int jobTypeId(JobType jobType){
    switch(jobType){
        case JobType::SPLIT:
            return 0;
        case JobType::FILTER:
            return 1;
        case JobType::NEIGHBOURS:
            return 2;
        case JobType::COMPONENTS:
            return 3;
        case JobType::CONTRACTION:
            return 4;
        case JobType::ANALYSIS:
            return 5;
        case JobType::MERGE:
            return 6;
        default:
            return -1;
    }
}

static bool operator<(JobType lhs, JobType rhs) noexcept {
    return jobTypeId(lhs) < jobTypeId(rhs);
}

enum class JobState{
    RUNNABLE,RUNNING,SUCCEED,FAILED,UNDEFINED,ABORTED
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

namespace std {
    template<>
    struct hash<Job>{
        size_t operator()(const Job & job) const noexcept {
            return job.id;
        }
    };
}

struct ConfigurableJob:public Job{
    std::filesystem::path config;
};

struct FilterJob: public ConfigurableJob {
    std::filesystem::path input;
};

struct NeighboursJob: public ConfigurableJob {
    std::filesystem::path primaryInput;
    std::vector<std::filesystem::path> additionalInput;
};

struct ComponentsJob:public ConfigurableJob {
    std::filesystem::path jobDirectory;
    size_t nextJobId;
};

struct ContractionJob: public ConfigurableJob{
    std::vector<std::filesystem::path> inputs;
    std::vector<uint64_t> components;
    std::string outputStem;
};

struct AnalysisJob: public ConfigurableJob {
    std::filesystem::path input;
    std::string outputStem;   
};

struct MergeJob: public Job {
    MergeJob(){
        this->type = JobType::MERGE;
    }
    std::vector<std::filesystem::path> inputs;
    std::filesystem::path output;
};

struct SplitJob: public Job {
    SplitJob() {
        this->type = JobType::SPLIT;
    }
    std::filesystem::path input;
    std::filesystem::path outDir;
    u_int32_t splits;
    int xOffset;
    int yOffset;
};