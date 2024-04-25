//
// Created by lolo on 09.04.24.
//

#include "SettlementFilterTask.h"
#include <fishnet/PathHelper.h>
#include <fishnet/StopWatch.h>
#include <fishnet/GISFactory.hpp>
#include <fishnet/StopWatch.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <CLI/CLI.hpp>

#include "FilterConfigJsonReader.hpp"
/**
 * MIT License Copyright (c) 2013-2022 Niels Lohmann

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 */

/**
 * CLI11 2.2 Copyright (c) 2017-2024 University of Cincinnati, developed by Henry Schreiner under NSF AWARD 1414736. All rights reserved.
 *
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

using json=nlohmann::json;
using namespace fishnet;


constexpr static const char * OUTPUT_SUFFIX = "_filtered.shp";

/**
 * 
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char * argv[]){
    CLI::App app{"FilterTask"};
    std::string inputFilename = "/home/lolo/Documents/fishnet/data/output/small_dataset/Punjab_Small.shp";
    std::string configFilename = "/home/lolo/Documents/fishnet/app/settlement_delineation_pattern_analysis/1_filter/config/filter.json";
    std::string outputDirectory;
    app.add_option("-i,--input",inputFilename,"Input GIS file for the filter step")->required()->check(CLI::ExistingFile);
    app.add_option("-c,--config", configFilename, "Path to filter.json file")->required()->check(CLI::ExistingFile);
    app.add_option("-o,--output", outputDirectory, "Output directory path")->check(CLI::ExistingDirectory);
    CLI11_PARSE(app,argc,argv);
    auto inputShapefile = GISFactory::asShapefile(inputFilename);
    auto taskExpected = inputShapefile.transform([&outputDirectory](const auto & input){
        std::filesystem::path outPath = std::filesystem::path(outputDirectory) / std::filesystem::path(input.getPath().stem().string() + OUTPUT_SUFFIX);
        Shapefile output = Shapefile(outPath);
        return SettlementFilterTask<fishnet::geometry::Polygon<double>>::create(input, output);
    });
    if(not taskExpected)
        std::cerr << taskExpected.error() << std::endl;
    auto & task = *taskExpected;
    std::cout << task.getInput().getPath() << std::endl;
    std::cout << task.getOutput().getPath() << std::endl;
    auto jsonReader = FilterConfigJsonReader::createFromPath(std::filesystem::path(configFilename));
    if(not jsonReader)
        std::cerr << jsonReader.error() << std::endl;
    auto success = jsonReader->read(task);
    if(not success)
        std::cerr << success.error() << std::endl;
    StopWatch filterWatch {task.getTaskName()};
    task.run();
    return 0;
}