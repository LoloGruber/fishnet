#pragma once
#include "JobDAG.hpp"
#include "CwlToolExecutor.hpp"

class Scheduler {
private:
    JobDAG_t dag;
    CwlToolExecutor executor;
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

public:
    Scheduler(JobDAG_t && dag,JobType lastJobType):dag(std::move(dag)),executor(),lastJobType(lastJobType){
        executor.setCallback(
            [this](Job & job){
                persistJobState(job);
                if(job.state == JobState::SUCCEED){
                     this->schedule();
                }
            }
        );
    }

    void schedule() {
        auto jobsToSchedule =  dag.getNodes() | std::views::filter([this](Job & job){return canBeScheduled(job);});
        std::vector<Job> jobsToRun;
        for(auto && job : jobsToSchedule){
            updateJobState(job,JobState::RUNNING);
            jobsToRun.push_back(std::move(job));
        }
        for(auto & job: jobsToRun) {
            executor.run(job);
        }
    }

    JobDAG_t & getDAG() noexcept {
        return this->dag;
    }

    const JobDAG_t & getDAG() const noexcept {
        return this->dag;
    }
};
