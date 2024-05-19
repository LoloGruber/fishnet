#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>
#include "NeighbourPredicateType.hpp"

struct DistanceBiPredicate{
    double maxDistanceInMeters;

    bool operator()(fishnet::geometry::IPolygon auto const & lhs, fishnet::geometry::IPolygon auto const & rhs) const noexcept {
        auto [l,r] = fishnet::geometry::closestPoints(lhs,rhs);
        return fishnet::WGS84Ellipsoid::distance(l,r) < maxDistanceInMeters;
    }

    static NeighbouringPredicateType type() {
        return NeighbouringPredicateType::DistanceBiPredicate;
    }
};