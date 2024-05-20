#pragma once 
#include <fishnet/Graph.hpp>
#include <fishnet/VectorLayer.hpp>
#include "SettlementPolygon.hpp"

enum class CentralityMeasureType {
    DegreeCentrality
};

/**
 * @brief Type of the centrality measure functor.
 * Takes a const reference to a graph, a reference to the vector layer storing the geometries and a map from Fishnet_ID to geometry feature (later stored inside the layer)
 * Expects to perform the centrality measure on the graph and add a layer for the result to the vector layer. Moreover for each node the centrality value must be stored in the feature map
 * @tparam GraphType graph type
 * @tparam GeometryType geometry type
 */
template<fishnet::graph::Graph GraphType, typename GeometryType>
using CentralityMeasure_t = std::function<void(const GraphType &,fishnet::VectorLayer<GeometryType> &,std::unordered_map<size_t,fishnet::Feature<GeometryType>> &)>;
