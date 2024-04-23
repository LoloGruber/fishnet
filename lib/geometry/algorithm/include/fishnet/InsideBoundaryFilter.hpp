#pragma once
#include "Filter.hpp"
#include <fishnet/ShapeGeometry.hpp>
namespace fishnet::geometry{

struct ContainedOrInHoleFilter{
    static bool operator()(const IPolygon auto & p, const IPolygon auto & underTest) noexcept{
        return p != underTest and not p.contains(underTest) and not p.isInHole(underTest);
    }
};

}