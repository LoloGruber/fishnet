// //
// // Created by grube on 08.09.2021.
// //
/**
 * @brief 
 * @deprecated
 */
// #include <gdal_priv.h>
// #include "gdal.h"
// #include "cpl_conv.h" // for CPLMalloc()
// #include <gdal_alg.h>
// #include <ogr_core.h>
// #include "ogrsf_frmts.h"

// #include "utils/Converter/GeotifftoOGRDataset.h"

// #include "io/PathHelper.h"

// #include "graph/vertex/Settlement.h"
// #include "graph/SettlementFactory.h"
// #include "graph/Network.h"

// #include "visualization/NetworkVisualization.h"
// #include "graph/centrality/IntegerCentralityMeasures/DegreeCentrality.h"

// #include <iostream>
// #include <boost/filesystem/path.hpp>
// #include "io/File/Shapefile.h"
// #include "utils/Geography/GeoUtil.h"
// #include "io/Parameters.h"
// #include "utils/Geometry/CharateristicShape/CharacteristicShape.h"

// #include "WSFNetworkApplication.h"

// int main(int argc, char *argv[]) {
//     GDALAllRegister(); //required
//     WSFNetworkApplication app = WSFNetworkApplication(argc,argv);
//     app.run();
//     return 0;
//     // PathHelper* tmp = PathHelper::instantiate(boost::filesystem::initial_path());
//     // /* Parse command line parameters using Parameters class, which stores configuration internally*/
//     // auto params = Parameters::create(argc, argv);

//     // /* If programm is not able to execute (probably due to missing file for <WSFArea>) -> terminate*/
//     // if (not params.execute()) {
//     //     return 0;
//     // }


//     // auto areaFile = params.getAreaFile();
//     // auto imperviousnessFile = params.getImpFile();
//     // auto populationFile = params.getPopFile();

//     // /* Initialize datasets from GISFile-> Geotiff files are converted, Shp files are copied*/
//     // std::unique_ptr<OGRDatasetWrapper> area = areaFile->toDataset();
//     // std::unique_ptr<OGRDatasetWrapper> imperviousness = nullptr;
//     // std::unique_ptr<OGRDatasetWrapper> population = nullptr;
//     // if (imperviousnessFile != nullptr) {
//     //     imperviousness = imperviousnessFile->toDataset();
//     // }
//     // if (populationFile != nullptr) {
//     //     population = populationFile->toDataset();
//     // }

//     // OGRSpatialReference *ref = area->getSpatialRef();
//     // /* Create Settlements, finding respective imperviousness and population values*/
//     // std::vector<std::shared_ptr<Settlement>> settlements = SettlementFactory::create(area, imperviousness, population);
//     // /* Store Settlments in Network and initialize network with configuration (default or set by parameters)*/
//     // auto network = std::make_shared<Network>(settlements,params.getNetworkConfig());
//     // /* Connect all settlements with their most "important"(by <SettlementValue>) neighbors, maximum: --maxEdgesPerNode outedges*/
//     // network->createEdges();
//     // /* Contract all edges with distance < --maxMergeDistance, merge settlements with specific merge strategy -m, initialized in NetworkConfiguration */

//     // if (params.doMerge()) {
//     //     network->contract();
//     // }

//     // /* Compute centrality measures for each chosen CentralityID*/
//     // std::vector<std::shared_ptr<CentralityMeasure>> centralityMeasures = params.getCentralityMeasures();
//     // for (auto &c: centralityMeasures) {
//     //     c->compute(network);
//     // }

//     // /* Visualize network, output stored at -o <OutputPath> or in current working directory*/
//     // auto visualization = NetworkVisualization(network,centralityMeasures, ref);
//     // std::shared_ptr<Shapefile> output = visualization.create(*params.getOutputPath());

//     // delete (tmp); // activate deconstructor of PathUtils-> deleting all temporary files

//     // std::cout << std::endl << "Output available at " <<output->getPath().string() << std::endl;
//     // return 0;
// }
#include <memory>
#include "Graph.h"
#include "UndirectedGraph.h"
#include <iostream>
#include "Polygon.h"
#include "StopWatch.h"




std::vector<Polygon> getRandomPolygons(int amount, int minNodes = 4, int maxNodes = 25){
    std::vector<Polygon> out; 
    for(int i = 0; i < amount; i++) {
        
    }
} 


int main(int argc, char * argv[]){
/*     auto g = graph::UndirectedGraph<int,int>();
    int i = 1;
    g.addNode(i);
    g.addNode(2);
    g.addEdge(3,4);
    g.addNode(1);
    for(auto x: g.getNodes()) {
        std::cout << x << std::endl;
    } */

}