//
// Created by grube on 09.01.2022.
//

#include <iostream>
#include <sstream>
#include "Parameters.h"
#include "graph/centrality/DoubleCentralityMeasures/MeanLocalSignificance.h"
#include "graph/centrality/IntegerCentralityMeasures/DegreeCentrality.h"
#include "graph/contraction/PolygonMergeStrategies/TwoShortestLinesMergeStrategy.h"
#include "graph/edge/Weight/DistanceWeight.h"
#include "graph/edge/Weight/NeighboringRelevance.h"
#include "io/File/Shapefile.h"
#include "io/File/GeoTIFF.h"
#include "ProgressPrinter.h"

bool Parameters::mapsInitialized = false;
Parameters::MergeMap Parameters::mergeStrategies = MergeMap();
Parameters::WeightMap Parameters::weightStrategies = WeightMap();
Parameters::CentralityMap Parameters::centralityMeasures = CentralityMap();

Parameters::Parameters(std::vector<std::string> &args) {
    if (not mapsInitialized) {
        initMaps();
    }
    this->args = args;
    if (args.empty()) {
        help();
    } else {
        parse();
    }
}

Parameters Parameters::create(int argc, char **args) {
    std::vector<std::string> stringArgs;
    if (argc <= 1) {
        return Parameters(stringArgs);
    }
    for (int i = 1; i < argc; i++) {
        stringArgs.emplace_back(args[i]);
    }
    return Parameters(stringArgs);
}

std::unique_ptr<GISFile> Parameters::getAreaFile() {
    if (this->wsf_area == nullptr) {
        //std::cerr << "GISFile provided for <WSFArea> cannot be null!" <<std::endl;
        return nullptr;
    }
    if (wsf_area->extension().string() == ".shp") {
        return std::make_unique<Shapefile>(*wsf_area, WSFTypeEnum::AREA);
    } else if (wsf_area->extension().string() == ".tif") {
        return std::make_unique<GeoTIFF>(*wsf_area, WSFTypeEnum::AREA);
    } else {
        //std::cerr << "GISFile provided for <WSFArea> cannot be null!" << std::endl;
        return nullptr;
    }

}

std::unique_ptr<GISFile> Parameters::getImpFile() {
    if (this->wsf_imp == nullptr) {;
        return nullptr;
    }
    if (wsf_imp->extension().string() == ".shp") {
        return std::make_unique<Shapefile>(*wsf_imp, WSFTypeEnum::IMPERVIOUSNESS);
    } else if (wsf_imp->extension().string() == ".tif") {
        return std::make_unique<GeoTIFF>(*wsf_imp, WSFTypeEnum::IMPERVIOUSNESS);
    } else {
        return nullptr;
    }


}

std::unique_ptr<GISFile> Parameters::getPopFile() {
    if (this->wsf_pop == nullptr) {;
        return nullptr;
    }
    if (wsf_pop->extension().string() == ".shp") {
        return std::make_unique<Shapefile>(*wsf_pop, WSFTypeEnum::POPULATION);
    } else if (wsf_pop->extension().string() == ".tif") {
        return std::make_unique<GeoTIFF>(*wsf_pop, WSFTypeEnum::POPULATION);
    } else {
        return nullptr;
    }
}

Parameters::PathPointer Parameters::getOutputPath() {
    if (output != nullptr) {
        auto p = std::make_unique<boost::filesystem::path>(*output);
        return std::move(p);
    } else {
        if(wsf_area != nullptr) {
            auto parent = wsf_area->parent_path();
            auto filename = boost::filesystem::path(wsf_area->stem().string() + "_Network.shp");
            auto path = parent / filename;
            int i = 1;
            while (boost::filesystem::exists(path)) {
                filename = boost::filesystem::path(
                        wsf_area->stem().string() + "_Network_" + std::to_string(i) + ".shp");
                path = parent / filename;
                i++;
            }
            return std::make_unique<boost::filesystem::path>(parent / filename);
        } else {
            auto parent = PathHelper::WORKING_DIR_PATH();
            auto filename = boost::filesystem::path("WSF_Network.shp");
            auto path = parent / filename;
            int i = 1;
            while (boost::filesystem::exists(path)) {
                filename = boost::filesystem::path("WSF_Network_" + std::to_string(i) + ".shp");
                path = parent / filename;
                i++;
            }
            return std::make_unique<boost::filesystem::path>(path);
        }
    }
}

NetworkConfiguration & Parameters::getNetworkConfig() {
    return this->networkConfiguration;
}

std::vector<std::shared_ptr<CentralityMeasure>> Parameters::getCentralityMeasures() {
    return this->selectedCentralityMeasures;
}


bool Parameters::execute(){
    if (wsf_area == nullptr) {
        std::cerr << "GISFile provided for <WSFArea> cannot be null!" <<std::endl;
        return false;
    }
    if (not interrupt) {
        this->toString();
        if (interactive) {
            std::cout << "Start Execution? (y/N) ";
            auto answer = std::cin.get();
            if(answer != 'y') {
                return false;
            }
            std::cout << std::endl;
        }
        if (silent) {
            std::cout.setstate(std::ios_base::failbit);
            ProgressPrinter::disable = true;
        }
        return true;
    }
    return false;
}

void Parameters::parse() {
    auto sIt = args.begin();
    wsf_area = pathParser(sIt);
    for (; sIt != args.end(); ++sIt) {
        auto s = *sIt;
        if (s == "-h" or s == "--help") {
            help();
            break;
        } else if (s == "-i" and (sIt + 1) != args.end()) {
            wsf_imp = pathParser(++sIt);
        } else if (s == "-p" and (sIt + 1) != args.end()) {
            wsf_pop = pathParser(++sIt);
        } else if (s == "-o" and (sIt + 1) != args.end()) {
            auto path = boost::filesystem::path(*(sIt + 1));
            auto parent = PathHelper::toAbsolute(boost::filesystem::path(path.parent_path())); //get absolute Path from provided string
            if (parent) {
                auto p = boost::filesystem::path(*parent / path.filename());
                int i = 1;
                auto filename = boost::filesystem::path(p.filename());
                /*Change version number if path already exists*/
                while (boost::filesystem::exists(p)) {
                    auto newFilename = boost::filesystem::path(
                            filename.stem().string() + std::to_string(i) + ".shp");
                    p = *parent / newFilename;
                    i++;
                }
                output = std::make_unique<boost::filesystem::path>(p);
            }
        } else if (s == "-m" or s == "--merge") {
            contract = true;
            if ((sIt + 1) != args.end()) {
                try {
                    int mergeId = std::stoi(*(sIt + 1));
                    if (mergeStrategies.contains(mergeId)) {
                        this->networkConfiguration.mergeStrategy = std::move(mergeStrategies[mergeId].first);
                    } else {
                        std::cerr << "MergeStrategyID " << mergeId << " does not exists!\n\n" << std::endl;
                        help();
                        break;

                    }
                } catch (std::exception &e) {
                    std::cerr << "MergeStrategyId is not a number!\n\n" << std::endl;
                    help();
                    break;
                }
            } else {
                std::cerr << "No merge option provided after -m or -merge!\n\n" << std::endl;
                help();
                break;
            }
        } else if (s == "-w" or s == "--weight") {
            if ((sIt + 1) != args.end()) {
                try {
                    int weighId = std::stoi(*(sIt + 1));
                    if (weightStrategies.contains(weighId)) {
                        this->networkConfiguration.weightStrategy = std::move(weightStrategies[weighId].first);
                    } else {
                        std::cerr << "WeightStrategyID " << weighId << " does not exists!\n\n" << std::endl;
                        help();
                        break;

                    }
                } catch (std::exception &e) {
                    std::cerr << "WeightStrategyID is not a number!\n\n" << std::endl;
                    help();
                    break;
                }
            } else {
                std::cerr << "No merge option provided after -w or --weight!\n\n" << std::endl;
                help();
                break;
            }
        } else if (s == "-c" or s == "--centrality") {
            if ((sIt + 1) != args.end()) {
                std::stringstream stringstream(*(sIt + 1));
                char delimeter = ',';
                std::string current;
                while (std::getline(stringstream, current, delimeter)) {
                    try {
                        int centralityID = std::stoi(current);
                        if (centralityMeasures.contains(centralityID)) {
                            auto centralityMeasure = std::move(centralityMeasures[centralityID].first);
                            this->selectedCentralityMeasures.push_back(centralityMeasure);
                        } else {
                            std::cerr << "CentralityID " << centralityID << " does not exists!\n\n" << std::endl;
                            help();
                            break;

                        }
                    } catch (std::exception &e) {
                        std::cerr << "CentralityID is not a number!\n\n" << std::endl;
                        help();
                        break;
                    }
                }
            } else {
                std::cerr << "No merge option provided after -w or --weight!\n\n" << std::endl;
                help();
                break;
            }
        } else if (s == "--maxEdgesPerNode") {
            if ((sIt + 1) != args.end()) {
                try {
                    int maxEdges = std::stoi(*(sIt + 1));
                    this->networkConfiguration.EDGES_PER_NODE = maxEdges;
                } catch (std::exception &exception) {
                    std::cerr << "--maxEdgesPerNode has to be a number!\n\n" << std::endl;
                    help();
                    break;
                }
            } else {
                std::cerr << "No value provided after --maxEdgesPerNode!\n\n" << std::endl;
                help();
                break;
            }
        } else if (s == "--maxMergeDistance") {
            if ((sIt + 1) != args.end()) {
                try {
                    int maxMergeDistance = std::stoi(*(sIt + 1));
                    this->networkConfiguration.MERGE_DISTANCE = (double) maxMergeDistance;
                } catch (std::exception &exception) {
                    std::cerr << "--maxMergeDistance has to be a number!\n\n" << std::endl;
                    help();
                    break;
                }
            } else {
                std::cerr << "No value provided after --maxMergeDistance!\n\n" << std::endl;
                help();
                break;
            }
        } else if (s == "--maxEdgeDistance") {
            if ((sIt + 1) != args.end()) {
                try {
                    int distance = std::stoi(*(sIt + 1));
                    this->networkConfiguration.CONNECT_DISTANCE = (double) distance;
                } catch (std::exception &exception) {
                    std::cerr << "--maxEdgeDistance has to be a number!\n\n" << std::endl;
                    help();
                    break;
                }
            } else {
                std::cerr << "No value provided after --maxEdgeDistance!\n\n" << std::endl;
                help();
                break;
            }
        } else if (s == "-s" or s == "--silent") {
            silent = true;
        } else if (s == "--disableMerge") {
            contract = false;
        } else if (s == "-n") {
            interactive = false;
        }
    }
}

void Parameters::help() {
    std::string delimeter = "\t-----------------------------------------------------------------------------------------------------------------------------------------------------------\n";
    std::string helpString = "World Settlement Footprint Urban Network Visualizer \n\nUsage \n "
                             "\t wsf_network <WSFPath> [Options]\n"
                             "\t wsf_network <WSFPath> -i <WSFImperviousnessPath> -p <WSFPopulationPath> -o <OutputPath>\n\n"
                             "All Paths have to be absolute paths or relative to the working directory of the executable!\n"
                             "Paths containing a whitespace have to be put in qoutes \"<Path>\"\n"
                             "<WSFPath> is mandatory for execution, -i and -p are optional, <OutputPath> defaults to current working directory\n\n"
                             "Options: \n\n\t";
    helpString.append(*getCentralityHelpString());
    helpString.append(delimeter);
    helpString.append("\t");
    helpString.append(*getMergeHelpString());
    helpString.append(delimeter);
    helpString.append("\t");
    helpString.append(*getWeightHelpString());
    helpString.append(delimeter);
    helpString.append("\t--maxEdgesPerNode [int] \t Set maximum of outgoing edges per node\n"
                      "\t--maxMergeDistance [int] \t maximum merge distance in meters (approximated)\n"
                      "\t--maxEdgeDistance [int] \t maxmimum distance for edges in meters(approximated)\n"
                      "\t--disableMerge\t\t Disable Edge Contraction and merging of settlements\n\n"
                      "\t-s \t\t\t\t disable console output\n\t--silent \t\t\t disable console output\n"
                      "\t-n \t\t\t\t non-interactive mode -> starts automatically\n\n"
                      );

    std::cout << helpString << std::flush;
    interrupt = true;
}


const std::string *Parameters::getCentralityHelpString() {
    auto * helpString= new std::string("CentralityMeasures:\n\n\t-c <CentralityID1>,<CentralityID2>,<...> \t\tSelect centrality measures by their code (Default = None)\n\t--centrality <CentralityID1>,<CentralityID2>,<...> \t[Example]: -c 0,1,3\n\n");
    helpString->append("\t\t CentralityID\t\tDescription\n");
    for (auto &c: centralityMeasures) {
        helpString->append("\t\t\t").append(std::to_string(c.first)).append("\t\t").append(c.second.second).append("\n");
    }
    helpString->append("\n");
    return helpString;
}

const std::string * Parameters::getMergeHelpString() {
    auto* helpString = new std::string ("MergeStrategies:\n\n\t-m <MergeStrategyID> \t\t Select merge strategy by its code (Default = DISABLED)\n\t--merge <MergeStrategyID> \t Example -m 0\n\n");
    helpString->append("\t\t MergeStrategyID\tDescription\n");
    for (auto &m: mergeStrategies) {
        helpString->append("\t\t\t").append(std::to_string(m.first)).append("\t\t").append(m.second.second).append("\n");
    }
    helpString->append("\n");
    return helpString;
}

const std::string *Parameters::getWeightHelpString() {
    auto * helpString = new std::string("EdgeWeights:\n\n\t-w <EdgeWeightID> \t\tSelect Edge Weight by its number code (Default = 0)\n\t--weight <EdgeWeightID> \tExample -w 0\n\n");
    helpString->append("\t\t EdgeWeightID\t\tDescription\n");
    for (auto &w: weightStrategies) {
        helpString->append("\t\t\t").append(std::to_string(w.first)).append("\t\t").append(w.second.second).append("\n");
    }
    helpString->append("\n");
    return helpString;
}

Parameters::PathPointer Parameters::pathParser(
        __gnu_cxx::__normal_iterator<std::vector<std::basic_string<char>>::pointer, std::vector<std::basic_string<char>>> iterator) {
    if (iterator != args.end()) {
        auto input = *iterator;
        return PathHelper::toAbsolute(input);
    }
    return PathHelper::toAbsolute("");
}

void Parameters::initMaps() {
    Parameters::centralityMeasures = {
            {0,std::make_pair(std::make_shared<DegreeCentrality>(),"Degree Centrality = number of edges pointing to a vertex ")},
            {1, std::make_pair(std::make_shared<MeanLocalSignificance>(),"Mean Local Significance Logarithmic = Log10 Average Local Significance with local significance of settlements u and v = u.Area * v.Area / Distance(u,v)^2 ")}
    };

    std::unique_ptr<PolygonMergeStrategy> polygonMerge = std::make_unique<CharacteristicPolygonMergeStrategy>();
    std::unique_ptr<AttributeMergeStrategy> attributeMerge = std::make_unique<DefaultAttributeMergeStrategy>();

    auto merge0 = std::make_pair(0, std::move(std::make_pair(std::move(std::make_unique<MergeStrategy>(attributeMerge,polygonMerge)),
                                                         "DEFAULT: PolygonMerge: Characteristic Shape\tAttributeMerge: DEFAULT")));
    Parameters::mergeStrategies.insert(std::move(merge0));

    auto w1 = std::make_pair(0, std::make_pair(std::make_unique<DistanceWeight>(),"Edge uv is weighted by the distance between u and v in meters"));
    auto w2 = std::make_pair(0, std::make_pair(std::make_unique<NeighboringRelevance>(),"Edge uv is weighted by the relevance of v (derived from attributes) and the inverse distance => high values for edges connecting near and relevant settlements"));

    Parameters::weightStrategies.insert(std::move(w1));
    Parameters::weightStrategies.insert(std::move(w2));
    Parameters::mapsInitialized = true;
}

void Parameters::toString(){
    std::string string = "Executing World Settlement Footprint Urban Network Visualizer with Parameters:\n";
    string.append("\tWSFPath = ");
    if (wsf_area) {
        string.append(wsf_area->string());
    } else {
        string.append("null");
    }
    string.append("\n\tWSFImperviousnessPath = ");
    if (wsf_imp) {
        string.append(wsf_imp->string());
    } else {
        string.append("null");
    }
    string.append("\n\tWSFPopulationPath = ");
    if (wsf_pop) {
        string.append(wsf_pop->string());
    } else {
        string.append("null");
    }
    string.append("\n\tOutputPath = ").append(getOutputPath()->string());
    string.append("\n\n\tCentralityMeasures = ");
    for (auto it = selectedCentralityMeasures.begin(); it != selectedCentralityMeasures.end();++it) {
        string.append((*it)->fieldName());
        if ((it + 1) != selectedCentralityMeasures.end()) {
            string.append(" ");
        }
    }
    string.append("\n\tMergeStrategy = ");
    if (not contract) {
        string.append("DISABLED");
    }else
    {
        string.append("DEFAULT");
    }
    string.append("\n\tWeightStrategy = ").append(networkConfiguration.weightStrategy->fieldName());
    string.append("\n\n\t--maxEdgesPerNode = ").append(std::to_string(networkConfiguration.EDGES_PER_NODE));
    string.append("\n\t--maxMergeDistance = ").append(std::to_string(networkConfiguration.MERGE_DISTANCE));
    string.append("\n\t--maxEdgeDistance = ").append(std::to_string(networkConfiguration.CONNECT_DISTANCE));
    string.append("\n\n");
    std::cout << string << std::flush;

}

bool Parameters::doMerge() const {
    return this->contract;
}




