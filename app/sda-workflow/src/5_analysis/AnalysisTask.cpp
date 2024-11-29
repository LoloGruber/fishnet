#include "AnalysisTask.h"
#include <fishnet/MultiPolygon.hpp>
#include <fstream>
#include <CLI/CLI.hpp>

int main(int argc, char * argv[]){
    using GeometryType = fishnet::geometry::MultiPolygon<fishnet::geometry::Polygon<double>>;
    CLI::App app {"AnalysisTask"};
    std::string inputFilename;
    std::string configFilename;
    std::string outputStem;
    std::string outputDirectory;
    size_t workflowID = 0;
    app.add_option("-w,--workflowID",workflowID,"Unique workflow id (optional)")->check(CLI::PositiveNumber);
    app.add_option("-i,--input",inputFilename,"Path to input shape file")->required()->check(CLI::ExistingFile);
    app.add_option("-c,--config",configFilename,"Path to configuration file")->required()->check(CLI::ExistingFile);
    app.add_option("--outputStem",outputStem,"Output file path, storing the merged polygons after performing the contraction on all inputs")->required();
    app.add_option("--outputDir",outputDirectory,"Output directory, created file at this directory with the filename ${outputStem}.shp")->check(CLI::ExistingDirectory);
    CLI11_PARSE(app,argc,argv);
    auto input = fishnet::Shapefile(inputFilename);
    auto outputPath = std::filesystem::path(outputDirectory) / std::filesystem::path(outputStem+".shp");
    auto output = fishnet::Shapefile(outputPath);
    auto config = AnalysisConfig(json::parse(std::ifstream(configFilename)));
    AnalysisTask<GeometryType> task {std::move(config),input,output,workflowID};
    task.run();
    return 0;
}