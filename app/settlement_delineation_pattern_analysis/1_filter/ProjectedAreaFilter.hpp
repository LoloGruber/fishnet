#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>
#include "Filter.hpp"

class ProjectedAreaFilter {
private:
    double requiredSize; //  Area in [long * lat]
public:
    explicit ProjectedAreaFilter(double requiredSize): requiredSize(requiredSize){}

    bool operator()(const fishnet::geometry::IPolygon auto & p ) const noexcept {
        return fishnet::WGS84Ellipsoid::area(p) >= requiredSize;
    }

    UnaryFilterType getType() const noexcept {
        return UnaryFilterType::ProjectedAreaFilter;
    }
};
