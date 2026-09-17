#pragma once
#include <fishnet/IGeometry.hpp>
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
        for(const auto & anySegment : p.getBoundary().getSegments()){
            double squaredFactor = pow(fishnet::WGS84Ellipsoid::distance(anySegment.p(),anySegment.q()),2)/pow(anySegment.length(),2); // estimates a factor to convert from lon,lat to m² for the polygon in question
            double approxArea = areaInLongLat * squaredFactor;
            return approxArea >= requiredArea; // return on first segment, as all segments should be similar in length and thus the factor should be similar
        }
        return false; // a boundary without segments has no area to speak of
    }
};

