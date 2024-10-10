#include <sstream>
#include "ContractionTask.h"
#include <fishnet/Polygon.hpp>
#include <fishnet/Shapefile.hpp>
#include <CLI/CLI.hpp> //CLI11 2.2 Copyright (c) 2017-2024 University of Cincinnati, developed by Henry Schreiner under NSF AWARD 1414736. All rights reserved.

int main(int argc, const char * argv[]){
    using GeometryType = fishnet::geometry::Polygon<double>;
    CLI::App app {"ContractionTask"};
    std::vector<std::string> inputFilenames;// {{"/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/test/workingDirectory/Punjab_0_1_filtered.shp"}};
    std::vector<ComponentReference> components;// {{{138241},{138240}}};
    std::string configFilename;//="/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/cfg/contraction.json";
    std::string outputStem;//="Test.shp";
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
    app.add_option("-c,--config",configFilename,"Path to configuration file")->required()->check(CLI::ExistingFile);
    app.add_option("--outputStem",outputStem,"Output file path, storing the merged polygons after performing the contraction on all inputs")->required();
    app.add_option("--components",components,"Component ids of connected components to contract")->each([](const std::string & str){
        std::stringstream stringStream {str};
        ComponentReference compRef;
        decltype(compRef.componentId) id = 0;
        if(not stringStream >> id){
            throw CLI::ValidationError("Could not parse \""+str+"\" to a component id");
        }
    });
    app.add_option("--outputDir",outputDirectory,"Output directory, created file at this directory with the filename ${outputStem}.shp")->check(CLI::ExistingDirectory);
    CLI11_PARSE(app,argc,argv);
    if(inputFilenames.size() < 1)
        throw std::runtime_error("No input files provided");
    ContractionConfig<GeometryType> config {json::parse(std::ifstream(configFilename))};
    auto outputPath = std::filesystem::path(outputDirectory) / std::filesystem::path(outputStem+".shp");
    fishnet::Shapefile output {outputPath};
    ContractionTask<GeometryType> task {std::move(config),std::move(components),output};
    for(auto && input : inputFilenames) {
        task.addInput({input});
    }
    task.run();
    return 0;
}