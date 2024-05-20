#include "FindNeighboursTask.h"
#include <fishnet/Shapefile.hpp>
#include <CLI/CLI.hpp> //CLI11 2.2 Copyright (c) 2017-2024 University of Cincinnati, developed by Henry Schreiner under NSF AWARD 1414736. All rights reserved.

int main(int argc, char const *argv[]){ 
    using GeometryType = fishnet::geometry::Polygon<double>;
    CLI::App app {"NeighboursTask"};
    std::vector<std::string> inputFilenames;
    std::string configFilename;
    app.add_option("-i,--inputs",inputFilenames,"Input Shapefiles storing the settlements")->required()->each([](const std::string & str){
        try{
            auto file = fishnet::Shapefile(str);
            if(not file.exists())
                throw std::invalid_argument("File "+ file.getPath().string() + " does not exist");
        }catch(std::invalid_argument & error){
            throw CLI::ValidationError(error.what());
        }
    });
    app.add_option("-c,--config",configFilename,"Path to neighbours.json configuration")->required()->check(CLI::ExistingFile);
    CLI11_PARSE(app,argc,argv);
    FindNeighboursTask<GeometryType> task {FindNeighboursConfig<GeometryType>(json::parse(std::ifstream(configFilename)))};
    for(auto && filename : inputFilenames) {
        task.addShapefile(fishnet::Shapefile(filename));
    }
    task.run();
    return 0;
}