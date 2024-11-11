#include <CLI/CLI.hpp>
#include "Scheduler.hpp"
#include "SchedulerConfig.hpp"
#include "Task.hpp"
#include "MemgraphConnection.hpp"

int main(int argc, char * argv[]){
    CLI::App schedulerApp {};
    std::string configFilename;
    schedulerApp.add_option("-c,--config",configFilename,"Path to scheduler / workflow configuration file")->required()->check(CLI::ExistingFile);
    CLI11_PARSE(schedulerApp,argc,argv);
    auto cfg = SchedulerConfig(nlohmann::json::parse(std::ifstream(configFilename)));
    auto exp = MemgraphConnection::create(cfg.params).transform([](auto && conn){return JobAdjacency(std::move(conn));});
    auto && jobAdj = getExpectedOrThrowError(exp);
    auto jobDAG = loadDAG(std::move(jobAdj));
    Scheduler scheduler = cfg.getSchedulerWithExecutorType(std::move(jobDAG));
    scheduler.schedule();
}