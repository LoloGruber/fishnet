//
// Created by grube on 05.01.2022.
//

#ifndef BACHELORARBEIT_NETWORKVISUALIZATION_H
#define BACHELORARBEIT_NETWORKVISUALIZATION_H
#include <memory>
#include "graph/Network.h"
#include "utils/Geography/OGRDatasetWrapper.h"
#include "graph/centrality/CentralityMeasure.h"

/**
 * Class responsible for the visualization of the network
 */
class NetworkVisualization {
private:
    std::shared_ptr<Network> network; //network to be visualized
    OGRSpatialReference * spatialRef; //spatial reference for the geographic projection
    std::vector<std::shared_ptr<CentralityMeasure>> centralities; // centrality measures that were applied

    /**
     * Helper method to visualize settlemetns
     * @param settlements to be visualized
     * @param layer output
     */
    static void visualize(const Network::SettlementSet &settlements, OGRLayer * layer);

    /**
     * Helper method to visualize edges
     * @param edges to be visualized
     * @param layer output
     */
    static void visualize(const Network::EdgeSet &edges,OGRLayer * layer);

    /**
     * Create fields for all centrality measures used in the output layer
     * @param layer
     */
    void initCentralityFields(OGRLayer *layer);

    /**
     * Helper method to set all CentralityEntry Values on the current OGRFeature
     * @param entries
     * @param feature
     */
    static void setCentralityFields(const std::vector<std::unique_ptr<CentralityEntry>> &entries, OGRFeature * feature);

public:
    NetworkVisualization(std::shared_ptr<Network> & network,std::vector<std::shared_ptr<CentralityMeasure>> & centralities, OGRSpatialReference * spatialReference):network(network), spatialRef(spatialReference), centralities(centralities) {};

    /**
     * Create visualization
     * @param outputPath to store the visualization
     * @return Shapefile Wrapper of the output
     */
    std::shared_ptr<Shapefile> create(boost::filesystem::path &outputPath);



};



#endif //BACHELORARBEIT_NETWORKVISUALIZATION_H
