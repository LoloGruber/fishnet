#include "FindNeighboursTask.h"
#include <fishnet/GISFactory.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>
#include <CLI/CLI.hpp>
#include "NeighboursConfigJsonReader.hpp"



int main(int argc, char const *argv[]){ 
    CLI::App app {"NeighboursTask"};
    std::vector<std::string> inputFilesnames;
    std::string configFilename;
    app.add_option("-i,--inputs",inputFilesnames,"Input Shapefiles for finding neighbours")->required()->each([](const std::string & str){
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
    FindNeighboursTask<fishnet::geometry::Polygon<double>> task;
    for(auto && filename : inputFilesnames) {
        task.addShapefile(fishnet::Shapefile(filename));
    }
    NeighboursConfigJsonReader reader {std::filesystem::path(configFilename)};
    reader.parse(task);
    task.run();
    return 0;
}
