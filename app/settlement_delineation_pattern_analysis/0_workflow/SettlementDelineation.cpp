#include "SettlementDelineation.h"
#include <nlohmann/json.hpp>
#include <CLI/CLI.hpp>
#include <fishnet/GISFactory.hpp>



int main(int argc, char * argv[]){
    CLI::App app {"SettlementDelineation Workload-Generator and Scheduler"};
    std::string pathToCfg = "/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cfg/workflow.json";
    std::string inputDirectory = "/home/lolo/Documents/fishnet/data/WSF/2019/Bolivia";
    std::vector<std::filesystem::path> inputFiles;
    app.add_option("-c,--config",pathToCfg,"Path to workflow.json configuration")->required()->check(CLI::ExistingFile);
    app.add_option("-i,--inputDirectory",inputDirectory,"Path to input directory")->required()->check(CLI::ExistingDirectory);
    CLI11_PARSE(app,argc,argv);
    for(auto const& file : std::filesystem::directory_iterator(inputDirectory)){
        if(file.is_regular_file() && fishnet::GISFactory::getType(file).has_value()) {
            inputFiles.push_back(file.path());
        }
    }
    SettlementDelineationConfig config {json::parse(std::ifstream(pathToCfg))};
    SettlementDelineation task {std::move(config),std::move(inputFiles)};
    task.run();
    return 0;
}