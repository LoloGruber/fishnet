//
// Created by lolo on 18.04.24.
//
#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>
#include "Filter.hpp"
#include <cmath>
#include <optional>
#include <nlohmann/json.hpp>
#include <magic_enum.hpp>

using json = nlohmann::json;

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

    constexpr static UnaryFilterType type()  noexcept {
        return UnaryFilterType::ApproxAreaFilter;
    }

    constexpr json toJson() const noexcept {
        json output;
        output["name"] = magic_enum::enum_name(type());
        output["requiredArea"]=requiredArea;
        return output;
    }

    static std::optional<ApproxAreaFilter> fromJson(json const & filterDesc) {
        try{
            auto area = filterDesc.at("requiredArea");
            return ApproxAreaFilter(area.template get<double>());
        }catch(  const json::exception & e){
            return std::nullopt;
        }
    }
};

