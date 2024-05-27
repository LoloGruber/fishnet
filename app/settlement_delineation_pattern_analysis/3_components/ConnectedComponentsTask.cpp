#include "ConnectedComponentsTask.h"
#include <CLI/CLI.hpp>

int main(int argc, char * argv[]){
    CLI::App app {"ComponentsTask"};
    std::string configFilename = "/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cfg/components.json";
    app.add_option("-c,--config",configFilename,"Path to components.json configuration")->required()->check(CLI::ExistingFile);
    // CLI11_PARSE(app,argc,argv);
    ConnectedComponentsTask task {ConnectedComponentsConfig(json::parse(std::ifstream(configFilename)))};
    task.run();
    return 0;
}