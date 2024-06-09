#pragma once
#include "JobDAG.hpp"
#include "CwlToolExecutor.hpp"
#include "Executor.hpp"

class Scheduler {
private:
    mutable std::mutex mutex;
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
        std::lock_guard lock {this->mutex};
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
        persistJobState(job);
    }

    void persistJobState(const Job & job) const noexcept{
        std::lock_guard lock {this->mutex};
        this->getDAG().getAdjacencyContainer().updateJobState(job);
    }

    auto onFinishedCallback() noexcept{
        return [this](const Job & job){
            std::cout << "Finished Job " << job.id << " (" << job.file.filename() << ") with state: "<<magic_enum::enum_name(job.state) << std::endl;
            this->persistJobState(job);
        };
    }

    bool hasRunnableJobs() const noexcept {
        std::lock_guard lock {this->mutex};
        const MemgraphConnection & connection = this->dag.getAdjacencyContainer().getConnection();
        if(
            Query("MATCH (j:Job {state:\"RUNNABLE\"})")
            .line("RETURN j.id")
            .execute(connection)
        ){
            auto result = connection->FetchAll();
            return result.has_value() && result->size() > 0;
        }   
        return false;
    }

    bool jobsAreRunning() const noexcept {
        std::lock_guard lock {this->mutex};
        const MemgraphConnection & connection = this->dag.getAdjacencyContainer().getConnection();
        if(
            Query("MATCH (j:Job {state:\"RUNNING\"})")
            .line("RETURN j.id")
            .execute(connection)
        ){
            auto result = connection->FetchAll();
            return result.has_value() && result->size() > 0;
        }   
        return false;
    }
    
public:
    Scheduler(JobDAG_t && dag,Executor auto && executor,JobType lastJobType):dag(std::move(dag)),lastJobType(lastJobType){
         static_assert(std::convertible_to<decltype(this->onFinishedCallback()),Callback_t>);
         executor.setCallback(onFinishedCallback());
         this->executor = std::move(executor);
    }

    void schedule() {
        while(hasRunnableJobs() || jobsAreRunning()){
            std::vector<Job> jobsToRun;
            for(auto & job: dag.getNodes()){
                if(canBeScheduled(job)){
                    jobsToRun.push_back(job);
                    updateJobState(job,JobState::RUNNING);
                }
            }
            for(const auto & job: jobsToRun) {
                std::cout << "Starting Job "<< job.id << " (" << job.file.filename() <<")"<< std::endl;
                executor(job);
            }
            sleep(1);
        }
        std::cout <<"Shutting down scheduler" <<std::endl;
        std::cout << std::endl;
    }

    JobDAG_t & getDAG() noexcept {
        return this->dag;
    }

    const JobDAG_t & getDAG() const noexcept {
        return this->dag;
    }
};
