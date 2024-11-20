#include "SettlementDelineation.h"
#include <nlohmann/json.hpp>
#include <CLI/CLI.hpp>


int main(int argc, char * argv[]){
    CLI::App app {"SettlementDelineation Workload-Generator and Scheduler"};
    std::string pathToCfg = "/home/lolo/Documents/fishnet/prod/local/sda-workflow-local.json";
    std::string inputPath = "/home/lolo/Documents/fishnet/data/WSF/2019/Punjab";
    std::string outputFilename;
    std::string workingDirectoryName /* = "/home/lolo/Documents/fishnet/tests/workflow/test/workingDirectory" */;
    std::vector<std::filesystem::path> inputFiles;
    app.add_option("-c,--config",pathToCfg,"Path to workflow configuration")->required()->check(CLI::ExistingFile);
    app.add_option("-i,--input",inputPath,"Path to input directory")->required()->check(CLI::ExistingDirectory | CLI::ExistingFile);
    app.add_option("-d,--workingDirectory",workingDirectoryName,"Path to working directory [default: temporary directory]")->check(CLI::ExistingDirectory);
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
    CLI11_PARSE(app,argc,argv);
    if(not workingDirectoryName.empty()){
        fishnet::util::TemporaryDirectory::setTmpPrefix(fishnet::util::PathHelper::absoluteCanonical(workingDirectoryName)); // change prefix for tmp directory
    }
    SettlementDelineation task {json::parse(std::ifstream(pathToCfg)),{inputPath},{outputFilename},{pathToCfg}};
    task.run();
    return 0;
}