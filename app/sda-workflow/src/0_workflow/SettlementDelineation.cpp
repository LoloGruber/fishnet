#include "SettlementDelineation.h"
#include <nlohmann/json.hpp>
#include <CLI/CLI.hpp>


int main(int argc, char * argv[]){
    CLI::App app {"SettlementDelineation Workload-Generator and Scheduler"};
    std::string pathToCfg = "/home/lolo/Documents/fishnet/prod/local/sda-workflow-local.json";
    std::string inputPath = "/home/lolo/Documents/fishnet/data/samples/Corvara_IT.tiff";
    std::string outputFilename = "/home/lolo/Desktop/CorvaraAnalysis.shp";
    std::string observerFilename /* = "/home/lolo/Desktop/sda-observer.json" */;
    std::vector<std::filesystem::path> inputFiles;
    app.add_option("-c,--config",pathToCfg,"Path to workflow configuration")->required()->check(CLI::ExistingFile);
    app.add_option("-i,--input",inputPath,"Path to input directory")->required()->check(CLI::ExistingDirectory | CLI::ExistingFile);
    app.add_option("--listener",observerFilename,"Path to json file observing the DAG of the workflow");
    app.add_option("-o,--output",outputFilename,"Path where merged output will be stored (only applied if last job type is at least MERGE)")->required()->check([](const std::string & str){
        try{
            auto file = fishnet::Shapefile(str); // try to create shapefile to ensure the correct extension
            std::filesystem::path parentPath = std::filesystem::path(str).parent_path();
            if (str.empty()) {
                return std::string("Output path is empty");
            }
            // Try creating the parent directories (if they don't exist)
            if (!parentPath.empty() && !std::filesystem::exists(parentPath)) {
                std::filesystem::create_directories(parentPath); // Create missing directories
            }
            // Check if the directory creation succeeded or if the path is already valid
            if (std::filesystem::exists(parentPath) && !std::filesystem::is_directory(parentPath)) {
                return std::string("Path \""+str+ "\" is not a valid directory");
            }

            if(not std::filesystem::exists(parentPath))
                return std::string("Path to output file (\""+str+"\") does not exist");
            return std::string();
        }catch(std::invalid_argument & shpError){
            return std::string("Invalid output shapefile extension:\n"+str+"\n")+ shpError.what();
        }
        catch(std::filesystem::filesystem_error & fsError ){
            return std::string("Invalid output path:\n"+str+"\n")+ fsError.what();
        }
    });
    // CLI11_PARSE(app,argc,argv);
    SettlementDelineationConfig cfg {json::parse(std::ifstream(pathToCfg))};
    if(std::filesystem::exists(cfg.scheduler.workingDirectory)){
        fishnet::util::TemporaryDirectory::setTmpPrefix(cfg.scheduler.workingDirectory); // change prefix for tmp directory
    }
    std::optional<std::filesystem::path> observerFile {};
    if(not observerFilename.empty()) {
        observerFile = {observerFilename};
    }
    SettlementDelineation task {std::move(cfg),{inputPath},{outputFilename},{pathToCfg},observerFile};
    task.run();
    return 0;
}