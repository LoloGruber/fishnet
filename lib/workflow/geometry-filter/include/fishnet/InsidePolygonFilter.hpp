#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/ContainedOrInHoleFilter.hpp>

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
};