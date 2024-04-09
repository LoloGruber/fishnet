//
// Created by lolo on 09.04.24.
//

#include "SettlementFilterTask.h"
#include "PathHelper.hpp"
#include "StopWatch.h"
#include <nlohmann/json.hpp>
#include <fstream>
/**
 * MIT License

Copyright (c) 2013-2022 Niels Lohmann

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
 * The main method launching the filter workflow step expects exactly one argument, which is a valid path to the task specification json file
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char * argv[]){
    using json=nlohmann::json;
    using namespace fishnet;
    if(argc != 2){
        std::cerr << "ERROR: Could not find task specification file\nExpecting one command line argument, but was "<<std::to_string(argc)<< std::endl;
        return 1;
    }
    std::filesystem::path pathToTaskConfig = argv[1];
    if(not std::filesystem::exists(pathToTaskConfig)){
        std::cerr << "ERROR: Could not find task specification file\nFile\"" << pathToTaskConfig.string()
                  << "\" does not exist.";
        return 1;
    }
    json config = json::parse(std::ifstream(pathToTaskConfig));

//
//    if (input.exists()) {
//        Shapefile output = input.appendToFilename("_filtered");
//        SettlementFilterTask<geometry::Polygon<double>> task{input, output};
//        auto areaPredicate = [](const geometry::Polygon<double> & polygon){
//            return polygon.area() >= 0.000001;
//        };
//        task.addPredicate(areaPredicate).addBiPredicate(geometry::ContainedOrInHoleFilter());
//        StopWatch w{"Filtering..."};
//        task.run();
//        w.stopAndPrint();
//    }
    return 0;
}