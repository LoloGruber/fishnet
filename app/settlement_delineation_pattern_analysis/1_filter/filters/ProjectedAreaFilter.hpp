#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>
#include "Filter.hpp"

class ProjectedAreaFilter {
private:
    double requiredSize; //  Area in [m²]
public:
    explicit ProjectedAreaFilter(double requiredSize): requiredSize(requiredSize){}

    bool operator()(const fishnet::geometry::IPolygon auto & p ) const noexcept {
        return fishnet::WGS84Ellipsoid::area(p) >= requiredSize;
    }

    constexpr static UnaryFilterType type()  noexcept {
        return UnaryFilterType::ProjectedAreaFilter;
    }

    static std::expected<ProjectedAreaFilter,std::string> fromJson(json const & filterDesc) {
        try{
            auto area = filterDesc.at("requiredArea");
            return ProjectedAreaFilter(area.template get<double>());
        }catch(  const json::exception & e){
            return std::unexpected(e.what());
        }
    }
};

