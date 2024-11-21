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
    Scheduler scheduler = cfg.getSchedulerWithExecutorType(loadDAG(JobAdjacency(MemgraphConnection::create(cfg.params,workflowID).value_or_throw())));
    scheduler.schedule();
}