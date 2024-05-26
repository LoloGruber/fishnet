#include "ConnectedComponentsTask.h"
#include <CLI/CLI.hpp>

int main(int argc, char * argv[]){
    CLI::App app {"ComponentsTask"};
    std::string configFilename;
    app.add_option("-c,--config",configFilename,"Path to components.json configuration")->required()->check(CLI::ExistingFile);
    CLI11_PARSE(app,argc,argv);
    ConnectedComponentsTask task {ConnectedComponentsConfig(json::parse(std::ifstream(configFilename)))};
    task.run();
    return 0;
}