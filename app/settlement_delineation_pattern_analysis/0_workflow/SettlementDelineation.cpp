#include "SettlementDelineation.h"
#include <nlohmann/json.hpp>
#include <CLI/CLI.hpp>
#include <fishnet/GISFactory.hpp>



int main(int argc, char * argv[]){
    CLI::App app {"SettlementDelineation Workload-Generator and Scheduler"};
    std::string pathToCfg = "/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cfg/workflow.json";
    std::string inputDirectory = "/home/lolo/Documents/fishnet/data/testing/Punjab_Small_Split";
    std::string outputFilename;
    std::string workingDirectory;
    std::vector<std::filesystem::path> inputFiles;
    app.add_option("-c,--config",pathToCfg,"Path to workflow.json configuration")->required()->check(CLI::ExistingFile);
    app.add_option("-i,--inputDirectory",inputDirectory,"Path to input directory")->required()->check(CLI::ExistingDirectory);
    app.add_option("-d,--workingDirectory",workingDirectory,"Path to working directory [default: current directory]")->check(CLI::ExistingDirectory);
    app.add_option("-o,--output",outputFilename,"Path where merged output will be stored (only applied if last job type is at least MERGE)")->check([](const std::string & str){
        try{
            auto file = fishnet::Shapefile(str);
            return std::string();
        }catch(std::invalid_argument & error){
            return std::string("Invalid output path:\n"+str+"\n")+ error.what();
        }
    });
    CLI11_PARSE(app,argc,argv);
    std::filesystem::path workingDir;
    if(workingDirectory.empty())
        workingDir = std::filesystem::current_path().string();
    else 
        workingDir = {workingDirectory};
    std::filesystem::path output;
    if(outputFilename.empty())
        output = workingDir / std::filesystem::path("Result.shp");
    else   
        output = outputFilename;    
    SettlementDelineation task {json::parse(std::ifstream(pathToCfg)),fishnet::GISFactory::getGISFiles(inputDirectory),output,workingDir};
    task.run();
    return 0;
}