#include "FindNeighboursTask.h"
#include <fishnet/Shapefile.hpp>
#include <CLI/CLI.hpp> //CLI11 2.2 Copyright (c) 2017-2024 University of Cincinnati, developed by Henry Schreiner under NSF AWARD 1414736. All rights reserved.

int main(int argc, char const *argv[]){ 
    using GeometryType = fishnet::geometry::Polygon<double>;
    CLI::App app {"NeighboursTask"};
    std::string primaryInput;
    std::vector<std::string> additionalInputs;
    std::string configFilename;
    size_t workflowID = 0;
    app.add_option("-w,--workflowID",workflowID,"Unique workflow id (optional)")->check(CLI::PositiveNumber);
    app.add_option("-i,--input",primaryInput,"Primary input shapefile storing the settlements")->required()->check(CLI::ExistingFile);
    app.add_option("-a,--additional_input",additionalInputs,"Additional input shapefiles storing the settlements")->each([](const std::string & str){
        try{
            auto file = fishnet::Shapefile(str);
            if(not file.exists())
                throw std::invalid_argument("File "+ file.getPath().string() + " does not exist");
        }catch(std::invalid_argument & error){
            throw CLI::ValidationError(error.what());
        }
    });
    app.add_option("-c,--config",configFilename,"Path to configuration file")->required()->check(CLI::ExistingFile);
    CLI11_PARSE(app,argc,argv);
    FindNeighboursTask<GeometryType> task {FindNeighboursConfig(json::parse(std::ifstream(configFilename))),fishnet::Shapefile(primaryInput),workflowID};
    for(auto && filename : additionalInputs) {
        task.addShapefile(fishnet::Shapefile(filename));
    }
    task.run();
    return 0;
}