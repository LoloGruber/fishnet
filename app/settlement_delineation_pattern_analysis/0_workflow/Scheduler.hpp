#pragma once
#include "JobDAG.hpp"
#include "CwlToolExecutor.hpp"
#include "Executor.hpp"

class Scheduler {
private:
    JobDAG_t dag;
    Executor_t executor;
    JobType lastJobType;

    bool isFinished(const Job & job) const noexcept {
        return job.state == JobState::FAILED || job.state == JobState::SUCCEED;
    }

    bool isRunnable(const Job & job) const noexcept {
        return job.state == JobState::RUNNABLE;
    }

    bool canBeScheduled( Job & job) const noexcept {
        if(job.type > lastJobType){
            updateJobState(job,JobState::ABORTED);
            return false;
        }
        if(not isRunnable(job))
            return false;
        return this->dag.inDegree(job) == 0 || std::ranges::all_of(dag.getReachableFrom(job),[this](const Job & parent){return this->isFinished(parent);});
    }

    void updateJobState(Job & job, JobState newState)const noexcept {
        job.updateStatus(newState);
        this->getDAG().getAdjacencyContainer().updateJobState(job);
    }

    void persistJobState(const Job & job) const noexcept{
        this->getDAG().getAdjacencyContainer().updateJobState(job);
    }

    auto onFinishedCallback() noexcept{
        return [this](Job & job){
            this->persistJobState(job);
            if(job.state == JobState::SUCCEED)
                this->schedule();
        };
    }


public:
    Scheduler(JobDAG_t && dag,Executor auto && executor,JobType lastJobType):dag(std::move(dag)),lastJobType(lastJobType){
         static_assert(std::convertible_to<decltype(this->onFinishedCallback()),Callback_t>);
         executor.setCallback(onFinishedCallback());
         this->executor = std::move(executor);
    }

    void schedule() {
        auto jobsToSchedule =  dag.getNodes() | std::views::filter([this](Job & job){return canBeScheduled(job);});
        std::vector<Job> jobsToRun;
        for(auto && job : jobsToSchedule){
            updateJobState(job,JobState::RUNNING);
            jobsToRun.push_back(std::move(job));
        }
        for(auto & job: jobsToRun) {
            executor(job);
        }
    }

    JobDAG_t & getDAG() noexcept {
        return this->dag;
    }

    const JobDAG_t & getDAG() const noexcept {
        return this->dag;
    }
};
