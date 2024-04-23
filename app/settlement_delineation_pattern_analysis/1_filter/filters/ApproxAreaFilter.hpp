//
// Created by lolo on 18.04.24.
//
#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>
#include "Filter.hpp"
#include <cmath>

class ApproxAreaFilter{
private:
    double requiredSize; // Area in [m²]
public:
    explicit ApproxAreaFilter(double requiredSize):requiredSize(requiredSize){}

    bool operator() (const fishnet::geometry::IPolygon auto & p) const noexcept {
        double areaInLongLat = p.area();
        auto anySegment = *std::ranges::begin(p.getBoundary().getSegments());
        double squaredFactor = pow(fishnet::WGS84Ellipsoid::distance(anySegment.p(),anySegment.q()),2)/pow(anySegment.length(),2);
        double approxArea = areaInLongLat * squaredFactor;
        return approxArea >= requiredSize;
    }

    UnaryFilterType getType() const noexcept {
        return UnaryFilterType::ApproxAreaFilter;
    }
};
