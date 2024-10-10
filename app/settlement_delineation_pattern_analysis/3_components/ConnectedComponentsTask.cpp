#include "ConnectedComponentsTask.h"
#include <CLI/CLI.hpp>

int main(int argc, char * argv[]){
    CLI::App app {"ComponentsTask"};
    std::string configFilename;// = "/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cfg/components.json";
    std::string jobDirectory;// = "/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cwl/jobs/split";
    size_t jobIdCounter;// = 0;
    app.add_option("-c,--config",configFilename,"Path to configuration file")->required()->check(CLI::ExistingFile);
    app.add_option("-j,--jobDirectory",jobDirectory,"Path to directory where the jobs are stored")->required()->check(CLI::ExistingDirectory);
    app.add_option("-i,--id",jobIdCounter,"Next free job id")->required()->check(CLI::PositiveNumber);
    CLI11_PARSE(app,argc,argv);
    ConnectedComponentsTask task {ConnectedComponentsConfig(json::parse(std::ifstream(configFilename))),{jobDirectory},{configFilename},jobIdCounter};
    task.run();
    return 0;
}