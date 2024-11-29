#include <CLI/CLI.hpp>
#include "Scheduler.hpp"
#include "SchedulerConfig.hpp"
#include "Task.hpp"
#include "MemgraphConnection.hpp"

int main(int argc, char * argv[]){
    CLI::App schedulerApp {};
    std::string configFilename; 
    size_t workflowID = 0;
    schedulerApp.add_option("-w,--workflowID",workflowID,"Unique workflow id (optional)")->check(CLI::PositiveNumber);
    schedulerApp.add_option("-c,--config",configFilename,"Path to scheduler / workflow configuration file")->required()->check(CLI::ExistingFile);
    CLI11_PARSE(schedulerApp,argc,argv);
    auto cfg = SchedulerConfig(nlohmann::json::parse(std::ifstream(configFilename)));
    if(cfg.workingDirectory.empty()) {
        throw std::runtime_error("No working directory provide to continue the workflow execution. A cfg.json file is located at the working directory of each workflow run.");
    }
    std::filesystem::current_path(cfg.workingDirectory);
    Scheduler scheduler = cfg.getSchedulerWithExecutorType(loadDAG(JobAdjacency(MemgraphConnection::create(cfg.params,workflowID).value_or_throw())));
    auto runningJobs = scheduler.getDAG().getNodes() | std::views::filter([](const Job & job){return job.state == JobState::RUNNING || job.state == JobState::FAILED;});
    std::ranges::for_each(runningJobs,[&scheduler](Job & job){
        job.state = JobState::RUNNABLE;
        scheduler.getDAG().getAdjacencyContainer().updateJobState(job);
    });
    scheduler.schedule();
}