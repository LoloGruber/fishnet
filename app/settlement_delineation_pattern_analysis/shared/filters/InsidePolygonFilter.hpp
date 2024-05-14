#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/ContainedOrInHoleFilter.hpp>
#include "Filter.hpp"
#include <nlohmann/json.hpp>

using namespace fishnet::geometry;

class InsidePolygonFilter{
private:
    static inline ContainedOrInHoleFilter filter{};
public:
    static inline bool operator()(IPolygon auto const & lhs, IPolygon auto const & rhs ) noexcept {
        return filter(lhs,rhs);
    }

    static BinaryFilterType type() noexcept {
        return BinaryFilterType::InsidePolygonFilter;
    }

    static std::optional<InsidePolygonFilter> fromJson(const nlohmann::json & json) {
        return InsidePolygonFilter();
    }
    
};