#include "SettlementDelineation.h"
#include <nlohmann/json.hpp>



int main(int argc, char * argv[]){
    std::string pathToCfg = "/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cfg/workflow.json";
    std::string inputDirectory = "/home/lolo/Documents/fishnet/data/WSF/2019/Bolivia";
    std::vector<std::filesystem::path> inputFiles;
    for(auto const& file : std::filesystem::directory_iterator(inputDirectory)){
        auto ext = file.path().extension();
        if(file.is_regular_file() && (ext==".shp" || ext==".tif")) {
            inputFiles.push_back(file.path());
        }
    }
    SettlementDelineationConfig config {json::parse(std::ifstream(pathToCfg))};
    SettlementDelineation task {std::move(config),std::move(inputFiles)};
    task.run();
    return 0;
}