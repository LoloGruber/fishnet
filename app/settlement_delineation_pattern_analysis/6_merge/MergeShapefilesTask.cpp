#include "MergeShapefilesTask.h"
#include <CLI/CLI.hpp>
#include <fishnet/MultiPolygon.hpp>

int main(int argc, char * argv[]){
    using GeometryType = fishnet::geometry::MultiPolygon<fishnet::geometry::Polygon<double>>;
    using EdgeGeometryType = fishnet::geometry::SimplePolygon<double>;
    CLI::App app {"CombinationTask"};
    std::vector<std::string> inputFilenames;
    std::string outputFilename;
    app.add_option("-i,--inputs",inputFilenames,"Input Shapefiles storing the settlements / edges and their attributes")->required()->each([](const std::string & str){
        try{
            auto file = fishnet::Shapefile(str);
            if(not file.exists())
                throw std::invalid_argument("File "+ file.getPath().string() + " does not exist");
        }catch(std::invalid_argument & error){
            throw CLI::ValidationError(error.what());
        }
    });
    app.add_option("-o,--output",outputFilename,"Output file location")->required()->check([](const std::string & str){
        try{
            auto file = fishnet::Shapefile(str);
            std::filesystem::path parentPath = std::filesystem::path(str).parent_path();
            if(not std::filesystem::exists(parentPath))
                return std::string("Path to output file does not exist");
            return std::string();
        }catch(std::invalid_argument & error){
            return std::string("Invalid output path:\n"+str+"\n")+ error.what();
        }
    });
    CLI11_PARSE(app,argc,argv);
    if(inputFilenames.empty()){
        throw std::runtime_error("No input files provided");
    }
    std::vector<fishnet::Shapefile> inputs;
    std::vector<fishnet::Shapefile> edges;
    for(auto && inputFilename: inputFilenames) {
        if(inputFilename.ends_with("_edges.shp"))
            edges.emplace_back(std::move(inputFilename));
        else
            inputs.emplace_back(std::move(inputFilename));
    }
    if(not inputs.empty()){
        MergeShapefilesTask<GeometryType> task {"Combination Task",std::move(inputs),{outputFilename}};
        task.run();
    }
    if(not edges.empty()){
        MergeShapefilesTask<EdgeGeometryType> edgeTask{"Combination Task Edges",std::move(edges),fishnet::Shapefile(outputFilename).appendToFilename("_edges")};
        edgeTask.run();
    }
    return 0;
}