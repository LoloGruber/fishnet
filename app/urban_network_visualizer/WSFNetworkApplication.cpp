#include "WSFNetworkApplication.h"
#include "graph/SettlementFactory.h"
#include <graph/Network.h>
#include "visualization/NetworkVisualization.h"
#include "io/File/Shapefile.h"

WSFNetworkApplication::WSFNetworkApplication(int argc, char * argv[]){;
    this->tmp = PathHelper::instantiate(boost::filesystem::initial_path());/*Create Temporary Directory*/
    Parameters params = Parameters::create(argc,argv);/* Parse command line parameters using Parameters class, which stores configuration internally*/
    /* Get GISFile from paramaters*/
    auto areaFile = params.getAreaFile();
    auto impFile = params.getImpFile();
    auto popFile = params.getPopFile();
    if(areaFile == nullptr) {
        throw std::invalid_argument("GISFile provided for <WSFArea> cannot be null!");
        /* If programm is not able to execute (probably due to missing file for <WSFArea>) -> terminate*/
    }
    this->area = areaFile->toDataset();
    if(impFile) {
        this->imp = impFile->toDataset();
    }
    if(popFile) {
        this->pop = popFile->toDataset();
    }
    this->spatialRef = area->getSpatialRef();
    this->centralityMeasures = params.getCentralityMeasures();
    this->networkConfig = std::move(params.getNetworkConfig());
    this->output = params.getOutputPath();
}

void WSFNetworkApplication::run(){
    /* Create Settlements, finding respective imperviousness and population values*/
    auto settlements = SettlementFactory::create(this->area,this->imp,this->pop);
    /* Store Settlments in Network and initialize network with configuration (default or set by parameters)*/
    auto network = std::make_shared<Network>(settlements,this->networkConfig);
    /* Connect all settlements with their most "important"(by <SettlementValue>) neighbors, maximum: --maxEdgesPerNode outedges*/
    network->createEdges();
    if(this->merge) {
        /* Contract all edges with distance < --maxMergeDistance, merge settlements with specific merge strategy -m, initialized in NetworkConfiguration */
        network->contract();
    }
    /* Compute centrality measures for each chosen CentralityID*/
    for(auto &c: this->centralityMeasures) {
        c->compute(network);
    }
    /* Visualize network, output stored at -o <OutputPath> or in current working directory*/
    auto visualization = NetworkVisualization(network,this->centralityMeasures,this->spatialRef);
    auto result = visualization.create(*this->output);
    std::cout << std::endl << "Output available at " <<result->getPath().string() << std::endl;

}

WSFNetworkApplication::~WSFNetworkApplication(){
    delete(this->tmp);
}