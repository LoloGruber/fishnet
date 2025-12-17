#pragma once
#include <fishnet/ShapeGeometry.hpp>
namespace fishnet::geometry{
/**
 * @brief BiPredicate Functor, which emits false when the polygon under test (2nd parameter) is contained in the other polygon
 * 
 */
struct ContainedOrInHoleFilter{
    static bool operator()(const IPolygon auto & p, const IPolygon auto & underTest) noexcept{
        return p != underTest and not p.contains(underTest) and not p.isInHole(underTest);
    }
};

}