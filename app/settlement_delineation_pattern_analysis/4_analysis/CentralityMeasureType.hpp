#pragma once 
#include <fishnet/Graph.hpp>
#include <fishnet/VectorLayer.hpp>
#include "SettlementPolygon.hpp"

enum class CentralityMeasureType {
    DegreeCentrality
};

template<fishnet::graph::Graph GraphType, typename GeometryType>
using CentralityMeasure_t = std::function<void(const GraphType &,fishnet::VectorLayer<GeometryType> &,std::unordered_map<size_t,fishnet::Feature<GeometryType>> &)>;
