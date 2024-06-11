#pragma once
#include <atomic>
#include "JobDAG.hpp"
#include "CwlToolExecutor.hpp"
#include "Executor.hpp"


class Scheduler {
private:
    mutable std::mutex mutex;
    JobDAG_t dag;
    Executor_t executor;
    JobType lastJobType;
    std::atomic_uint16_t activeThreads = 0;

    static inline size_t THREAD_CONCURRENCY = std::thread::hardware_concurrency();

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
        if(not isRunnable(job) || activeThreads >= THREAD_CONCURRENCY)
            return false;
        return this->dag.inDegree(job) == 0 || std::ranges::all_of(dag.getReachableFrom(job),[this](const Job & parent){return this->isFinished(parent);});
    }

    void updateJobState(Job & job, JobState newState)const noexcept {
        if(job.state != newState){
            job.updateStatus(newState);
            persistJobState(job);
            printOnJobStateChange(job);
        }
    }

    void persistJobState(const Job & job) const noexcept{
        this->getDAG().getAdjacencyContainer().updateJobState(job);
    }

    void printOnJobStateChange(const Job & job) const noexcept {
        std::cout << magic_enum::enum_name(job.state) << ": Job "<<job.id << " (" << job.file.filename() << ")" << std::endl;
    }

    auto onFinishedCallback() noexcept{
        return [this](const Job & job){
            std::lock_guard lock {this->mutex};
            this->persistJobState(job);
            this->printOnJobStateChange(job);
            this->activeThreads--;
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
            {
                std::lock_guard lock {this->mutex};
                for(auto & job: dag.getNodes()){
                    if(canBeScheduled(job)){
                        activeThreads++;
                        jobsToRun.push_back(job);
                        updateJobState(job,JobState::RUNNING);
                    }
                }
            }
            for(const auto & job: jobsToRun) {
                executor(job);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
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
