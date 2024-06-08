#pragma once
#include "Executor.hpp"
#include <magic_enum.hpp>
#include <iostream>
#include <thread>

class CwlToilExecutor{
private:
    std::filesystem::path cwlDir;
    std::string flags;
    Callback_t callback;

    std::string getCwlFile(JobType type){
        auto jobTypeName = std::string(magic_enum::enum_name(type));
        std::transform(jobTypeName.begin(),jobTypeName.end(),jobTypeName.begin(),::tolower);
        auto pathToCwl = cwlDir / std::filesystem::path(jobTypeName+".cwl");
        return pathToCwl;
    }

public:
    CwlToilExecutor(std::filesystem::path && cwlDir, std::string && flags)
    :cwlDir(std::move(cwlDir)),flags(std::move(flags)){}

    void setCallback(Callback_t && callback){
        this->callback = std::move(callback);
    }

    void operator()(Job & job){
        std::thread([this,&job]()mutable{
            std::stringstream command;
            command << "toil-cwl-runner " << flags << " "<< getCwlFile(job.type) << " " << job.file.string() << std::endl;
            std::cout << "Starting Job "<< job.id << " (" << job.file <<")"<< std::endl;
            int exitCode = std::system(command.str().c_str());
            job.updateStatus(exitCode==0?JobState::SUCCEED:JobState::FAILED);
            std::cout << "Finished Job " << job.id << " with state: "<<magic_enum::enum_name(job.state) << std::endl;
            callback(job);
        }).detach();
    }
};
static_assert(Executor<CwlToilExecutor>);