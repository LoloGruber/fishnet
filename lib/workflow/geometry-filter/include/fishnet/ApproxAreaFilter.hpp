#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>
#include <cmath>

/**
 * @brief Filter that approximates the area in m² and checks if it is equal or greater than the required area
 * 
 */
class ApproxAreaFilter{
private:
    double requiredArea; // Area in [m²]
public:
    explicit ApproxAreaFilter(double requiredArea):requiredArea(requiredArea){}

    bool operator() (const fishnet::geometry::IPolygon auto & p) const noexcept {
        double areaInLongLat = p.area();
        auto anySegment = *std::ranges::begin(p.getBoundary().getSegments());
        double squaredFactor = pow(fishnet::WGS84Ellipsoid::distance(anySegment.p(),anySegment.q()),2)/pow(anySegment.length(),2); // estimates a factor to convert from lon,lat to m² for the polygon in question
        double approxArea = areaInLongLat * squaredFactor; 
        return approxArea >= requiredArea;
    }
};

