//
// Created by grube on 09.01.2022.
//

#ifndef BACHELORARBEIT_PARAMETERS_H
#define BACHELORARBEIT_PARAMETERS_H
#include <boost/filesystem/path.hpp>
#include "centrality/CentralityMeasure.h"
#include "graph/contraction/MergeStrategy.h"
#include "graph/edge/Weight/Weight.h"
#include "graph/NetworkConfiguration.h"
#include "io/File/GISFile.h"

/**
 * Class responsible for handling the command line parameters
 */
class Parameters {
public:
    /* Definition of type abbrevations*/
    using CentralityMap = std::unordered_map<int, std::pair<std::shared_ptr<CentralityMeasure>, std::string>>;
    using MergeMap = std::unordered_map<int, std::pair<std::unique_ptr<MergeStrategy>, std::string>>;
    using WeightMap = std::unordered_map<int, std::pair<std::unique_ptr<Weight>, std::string>>;
    using PathPointer = std::unique_ptr<boost::filesystem::path>;
    static bool mapsInitialized; //keeps track whether the maps storing all the possible Strategies are initialized

    /**
     * Initialize static maps, showing all the possible Weight, Merge and Centrality Strategies
     */
    static void initMaps();
private:
    // Key: Code of Centrality Measure      Value: (PointerToObject, Description)
    static CentralityMap centralityMeasures;

    // Key: Cde of Merge Strategy          Value: (PointerToMergeStrategy, Description)
    static MergeMap mergeStrategies;

    //Key: Code of Weight Strategy          Value: (PointerToWeightStrategy, Description)
    static WeightMap weightStrategies;


    std::vector<std::string> args; // arguments passed to the program
    bool interrupt = false; // set to true on invalid input, or --help => prevent execution of programm
    bool silent = false; //When true: no standard output
    bool contract = false; //When true: network should be contracted
    bool interactive = true; // Yes/no prompt active when true
    PathPointer wsf_area; //path to WSF-Area file
    PathPointer wsf_imp; //path to WSF-Imp file
    PathPointer wsf_pop; //path to WSF-Pop file
    PathPointer output; //Output Path
    std::vector<std::shared_ptr<CentralityMeasure>> selectedCentralityMeasures; //storing all the selected centrality measures

    NetworkConfiguration networkConfiguration = NetworkConfiguration(); //initialize NetworkConfiguration with default. Updated later when parameters are passed

    /**
     * Parser for command line parameters, sets private class member accordingly
     * */
    void parse();

    /**
     * Prints the chosen parameters
     */
    void toString();

    static const std::string *getCentralityHelpString();

    static const std::string *getMergeHelpString();

    static const std::string *getWeightHelpString();

    /**
     * Helper Method to retrieve path from args
     * @param iterator current position in args
     * @return PathToFile
     */
    PathPointer pathParser(
            __gnu_cxx::__normal_iterator<std::vector<std::basic_string<char>>::pointer, std::vector<std::basic_string<char>>> iterator);
public:

    explicit Parameters(std::vector<std::string> &args);

    std::unique_ptr<GISFile> getAreaFile();

    std::unique_ptr<GISFile> getImpFile();

    std::unique_ptr<GISFile> getPopFile();

    /**
     *
     * @return specified output path by the user or path in current working directory, with filename of wsf-area file followed by "_Network.shp"
     */
    PathPointer getOutputPath();

    NetworkConfiguration & getNetworkConfig();

    std::vector<std::shared_ptr<CentralityMeasure>> getCentralityMeasures();

    /**
     *
     * @return whether the program can be executed with the given parameters
     */
    bool execute();

    /**
     * Prints help string to the console
     */
    void help();

    /**
     * Factory Method to create instance of Parameters
     * @param argc argument count
     * @param args argument array from standard in
     * @return Parameter object initialized with args
     */
    static Parameters create(int argc, char * args[]);

    /**
     *
     * @return true if this->contract == true
     */
    bool doMerge() const;
};


#endif //BACHELORARBEIT_PARAMETERS_H
