#include "SettlementDelineation.h"
#include <nlohmann/json.hpp>
#include <CLI/CLI.hpp>
#include <fishnet/GISFactory.hpp>



int main(int argc, char * argv[]){
    CLI::App app {"SettlementDelineation Workload-Generator and Scheduler"};
    std::string pathToCfg = "/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cfg/workflow.json";
    std::string inputDirectory = "/home/lolo/Documents/fishnet/data/testing/Punjab_Split";
    std::string workingDirectory;
    std::vector<std::filesystem::path> inputFiles;
    app.add_option("-c,--config",pathToCfg,"Path to workflow.json configuration")->required()->check(CLI::ExistingFile);
    app.add_option("-i,--inputDirectory",inputDirectory,"Path to input directory")->required()->check(CLI::ExistingDirectory);
    app.add_option("-d,--workingDirectory",workingDirectory,"Path to working directory [default: current directory]")->check(CLI::ExistingDirectory);
    CLI11_PARSE(app,argc,argv);
    if(workingDirectory.empty())
        workingDirectory = std::filesystem::current_path().string();
    SettlementDelineation task {json::parse(std::ifstream(pathToCfg)),fishnet::GISFactory::getGISFiles(inputDirectory),{workingDirectory}};
    task.run();
    return 0;
}