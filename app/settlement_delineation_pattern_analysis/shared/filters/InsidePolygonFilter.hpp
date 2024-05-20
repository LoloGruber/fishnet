#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/ContainedOrInHoleFilter.hpp>
#include "Filter.hpp"
#include <nlohmann/json.hpp>

/**
 * @brief Binary Filter, testing if the other polygons contain another or are inside a hole
 * 
 */
class InsidePolygonFilter{
private:
    static inline fishnet::geometry::ContainedOrInHoleFilter filter{};
public:
    static inline bool operator()(fishnet::geometry::IPolygon auto const & lhs, fishnet::geometry::IPolygon auto const & rhs ) noexcept {
        return filter(lhs,rhs);
    }

    static BinaryFilterType type() noexcept {
        return BinaryFilterType::InsidePolygonFilter;
    }

    static std::optional<InsidePolygonFilter> fromJson(const nlohmann::json & json) {
        return InsidePolygonFilter();
    }
    
};