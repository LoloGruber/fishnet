#include "ContractionTask.h"
#include <fishnet/Polygon.hpp>
#include "DistanceBiPredicate.hpp"
#include <CLI/CLI.hpp>



int main(int argc, const char * argv[]){
    using GeometryType = fishnet::geometry::Polygon<double>;
    CLI::App app {"ContractionTask"};
    std::vector<std::string> inputFilenames;
    std::string configFilename;
    std::string outputStem;
    std::string outputDirectory;
    app.add_option("-i,--inputs",inputFilenames,"Input Shapefiles storing the polygons with id for the contraction")->required()->each([](const std::string & str){
        try{
            auto file = fishnet::Shapefile(str);
            if(not file.exists())
                throw std::invalid_argument("File "+ file.getPath().string() + " does not exist");
        }catch(std::invalid_argument & error){
            throw CLI::ValidationError(error.what());
        }
    });
    app.add_option("-c,--config",configFilename,"Path to contract.json configuration")->required()->check(CLI::ExistingFile);
    // app.add_option("-o,--output",outputStem,"Output file path, storing the merged polygons after performing the contraction on all inputs")->required()->check([](const std::string & str){
    //     try{
    //         auto asShp = fishnet::Shapefile(str);
    //         return std::string();
    //     }catch(std::invalid_argument & error){
    //         return std::string("Output filename is not valid\n")+error.what();
    //     }
    // });
    app.add_option("--outputStem",outputStem,"Output file path, storing the merged polygons after performing the contraction on all inputs")->required();
    app.add_option("--outputDir",outputDirectory,"Output directory, created file at this directory with the filename ${outputStem}.shp")->required()->check(CLI::ExistingDirectory);
    CLI11_PARSE(app,argc,argv);
    if(inputFilenames.size() < 1)
        throw std::runtime_error("No input files provided");
    ContractionConfig<GeometryType> config {json::parse(std::ifstream(configFilename))};
    auto outputPath = std::filesystem::path(outputDirectory) / std::filesystem::path(outputStem+".shp");
    std::cout << outputPath <<std::endl;
    fishnet::Shapefile output {outputPath};
    ContractionTask<GeometryType> task {std::move(config),output};
    for(auto && input : inputFilenames) {
        task.addInput({input});
    }
    config.contractBiPredicates.push_back(DistanceBiPredicate(500.0));
    config.workers = 1;
    task.run();
    return 0;
}