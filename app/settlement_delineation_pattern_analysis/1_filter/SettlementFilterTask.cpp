#include "SettlementFilterTask.h"
#include <fishnet/PathHelper.h>
#include <fishnet/StopWatch.h>
#include <fishnet/GISFactory.hpp>
#include <fishnet/StopWatch.h>
#include <fstream>
#include <nlohmann/json.hpp> //MIT License Copyright (c) 2013-2022 Niels Lohmann
#include <CLI/CLI.hpp> //CLI11 2.2 Copyright (c) 2017-2024 University of Cincinnati, developed by Henry Schreiner under NSF AWARD 1414736. All rights reserved.


#include "FilterConfig.hpp"

using json=nlohmann::json;
using namespace fishnet;

constexpr static const char * OUTPUT_SUFFIX = "_filtered.shp";

int main(int argc, char * argv[]){
    using PolygonType = fishnet::geometry::Polygon<double>;
    CLI::App app{"FilterTask"};
    std::string inputFilename;
    std::string configFilename;
    std::string outputDirectory;
    app.add_option("-i,--input",inputFilename,"Input GIS file for the filter step")->required()->check(CLI::ExistingFile);
    app.add_option("-c,--config", configFilename, "Path to filter.json file")->required()->check(CLI::ExistingFile);
    app.add_option("-o,--output", outputDirectory, "Output directory path")->check(CLI::ExistingDirectory);
    CLI11_PARSE(app,argc,argv);
    auto inputShapefile = GISFactory::asShapefile(inputFilename);
    auto taskExpected = inputShapefile.transform([&outputDirectory](const auto & input){
        std::filesystem::path outPath = std::filesystem::path(outputDirectory) / std::filesystem::path(input.getPath().stem().string() + OUTPUT_SUFFIX);
        Shapefile output = Shapefile(outPath);
        return SettlementFilterTask<PolygonType>::create(input, output);
    });
    auto & task = getExpectedOrThrowError(taskExpected);
    task.setConfig(FilterConfig<PolygonType>(json::parse(std::ifstream(configFilename))));
    task.run();
    return 0;
}