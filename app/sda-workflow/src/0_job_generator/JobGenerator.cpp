#include "JobGenerator.hpp"
#include <CLI/CLI.hpp>
#include <fishnet/GISFactory.hpp>
#include "MemgraphConnection.hpp"
#include "Task.hpp"

int main(int argc, char * argv[]){
    CLI::App generatorApp;
    std::string workingDirectoryName;
    std::string inputDirectory;
    std::string configPath;
    size_t workflowID = 0;
    generatorApp.add_option("-c,--config",configPath,"Path to job generator / workflow configuration file")->required()->check(CLI::ExistingFile);
    generatorApp.add_option("-i,--inputDirectory",inputDirectory,"Path to input directory of GIS Files")->required()->check(CLI::ExistingDirectory);
    generatorApp.add_option("-d,--workingDirectory",workingDirectoryName,"Working Directory for the workflow")->check(CLI::ExistingDirectory);
    generatorApp.add_option("-w,--workflowID",workflowID,"Unique workflow id (optional)")->check(CLI::PositiveNumber);
    CLI11_PARSE(generatorApp,argc,argv);
    auto cfg = JobGeneratorConfig(nlohmann::json::parse(std::ifstream(configPath)));
    std::filesystem::path workingDirectory;
    if(workingDirectoryName.empty()) {
        workingDirectory = std::filesystem::current_path();
    }else {
        workingDirectory = std::filesystem::path(workingDirectoryName);
    }
    auto exp = MemgraphConnection::create(cfg.params,workflowID).transform([](auto && conn){return JobAdjacency(std::move(conn));});
    auto && jobAdj = getExpectedOrThrowError(exp);
    auto jobDAG = loadDAG(std::move(jobAdj));
    JobGenerator jobGenerator {std::move(cfg),std::move(workingDirectory),{configPath}};
    jobGenerator.generate(fishnet::GISFactory::getGISFiles(inputDirectory),jobDAG);
    return 0;
}