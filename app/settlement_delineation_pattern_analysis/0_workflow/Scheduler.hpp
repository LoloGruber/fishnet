#pragma once
#include "JobDAG.hpp"
#include "CwlToolExecutor.hpp"

class Scheduler {
private:
    JobDAG_t dag;
    CwlToolExecutor executor;

    bool isFinished(const Job & job) const noexcept {
        return job.state == JobState::FAILED || job.state == JobState::SUCCEED;
    }

    bool isRunnable(const Job & job) const noexcept {
        return job.state == JobState::RUNNABLE;
    }

    bool canBeScheduled(const Job & job) const noexcept {
        if(not isRunnable(job))
            return false;
        return this->dag.inDegree(job) == 0 || std::ranges::all_of(dag.getReachableFrom(job),[this](const Job & parent){return this->isFinished(parent);});
    }



public:
    Scheduler(JobDAG_t && dag):dag(std::move(dag)),executor(){
        executor.setCallback(
            [this](Job & job){
                this->getDAG().getAdjacencyContainer().updateJobState(job);
                if(job.state == JobState::SUCCEED){
                     this->schedule();
                }
            }
        );
    }

    void schedule() {
        auto jobsToSchedule =  dag.getNodes() | std::views::filter([this](const Job & job){return canBeScheduled(job);});
        std::vector<Job> jobsToRun;
        for(auto && job : jobsToSchedule){
            job.updateStatus(JobState::RUNNING);
            dag.getAdjacencyContainer().updateJobState(job);
            jobsToRun.push_back(std::move(job));
        }
        for(auto & job: jobsToRun) {
            executor.run(job);
        }
    }

    JobDAG_t & getDAG() noexcept {
        return this->dag;
    }

    static auto jobFinishedCallback(Scheduler & scheduler,Job & job) noexcept {
        return [](Scheduler & scheduler,Job & job){
          
        };
    }


};
