#include "FindNeighboursTask.h"
#include <fishnet/GISFactory.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>
#include <CLI/CLI.hpp>



int main(int argc, char const *argv[]){ 
    using GeometryType = fishnet::geometry::Polygon<double>;
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
    FindNeighboursTask<GeometryType> task {FindNeighboursConfig<GeometryType>(json::parse(std::ifstream(configFilename)))};
    for(auto && filename : inputFilesnames) {
        task.addShapefile(fishnet::Shapefile(filename));
    }
    task.run();
    return 0;
}
