#pragma once
#include <sstream>
#include <filesystem>
#include "Job.hpp"
#include "JobAdjacency.hpp"

class CwlToolExecutor{
private:
    static inline std::filesystem::path cwlDir = "/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cwl";
    std::function<void(Job & job)> callback;
public:
    void setCallback(std::function<void(Job & job)> cb){
        this->callback = cb;
    }

    void run(Job & job){
        std::stringstream command;
        command << "cwltool ";
        auto jobTypeName = std::string(magic_enum::enum_name(job.type));
        std::transform(jobTypeName.begin(),jobTypeName.end(),jobTypeName.begin(),::tolower);
        auto pathToCwl = cwlDir / std::filesystem::path(jobTypeName+".cwl");
        command << pathToCwl << " " << job.file.string() << std::endl;
        int exitCode = system(command.str().c_str());
        job.updateStatus(exitCode==0?JobState::SUCCEED:JobState::FAILED);
        callback(job);
    }
};