#pragma once
#include <vector>
#include <fishnet/Shapefile.hpp>
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/VectorLayer.hpp>
#include <fishnet/PolygonNearestNeighbours.hpp>
#include "SettlementPolygon.hpp"


template<fishnet::geometry::IPolygon P>
class FindNeighboursTask{
private:
    std::vector<fishnet::Shapefile> inputs;

public:

    void run() {
        std::vector<SettlementPolygon<P>> polygons;
        for(const auto & shp : inputs) {
            auto layer = fishnet::VectorLayer<P>::read(shp);
            for(const auto & feature : layer.getFeatures()) {

            }
        }
        auto result = findNeighbouringPolygons(polygons,fishnet::util::TrueBiPredicate());
        
    }
};