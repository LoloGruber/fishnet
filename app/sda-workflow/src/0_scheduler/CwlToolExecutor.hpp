#pragma once
#include <sstream>
#include <filesystem>
#include "Job.hpp"
#include "Executor.hpp"

class CwlToolExecutor{
private:
    std::filesystem::path cwlDir = "/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cwl";
    std::string flags;
    Callback_t callback;
public:
    CwlToolExecutor(std::filesystem::path && cwlDir, std::string && flags):cwlDir(std::move(cwlDir)),flags(std::move(flags)){}

    void setCallback(Callback_t && callback){
        this->callback = std::move(callback);
    }

    void operator()(Job job)const{
        std::thread([this,job=std::move(job)]()mutable{
            std::stringstream command;
            command << "cwltool " << flags <<" ";
            auto jobTypeName = std::string(magic_enum::enum_name(job.type));
            std::transform(jobTypeName.begin(),jobTypeName.end(),jobTypeName.begin(),::tolower);
            auto pathToCwl = cwlDir / std::filesystem::path(jobTypeName+".cwl");
            command << pathToCwl << " " << job.file.string() << std::endl;
            int exitCode = system(command.str().c_str());
            job.updateStatus(exitCode==0?JobState::SUCCEED:JobState::FAILED);
            callback(job);
        }).detach();
    }
};
static_assert(Executor<CwlToolExecutor>);