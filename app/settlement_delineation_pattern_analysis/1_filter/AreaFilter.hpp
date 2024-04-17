#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>

class AreaFilter {
private:
    double requiredSize; //  Area in [long * lat]
public:
    explicit AreaFilter(double requiredSize):requiredSize(requiredSize){}

    bool operator()(const fishnet::geometry::IPolygon auto & p ) const noexcept {
        return fishnet::WGS84Ellipsoid::area(p) >= requiredSize;
    }


};
