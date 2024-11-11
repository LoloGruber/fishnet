#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>
#include "NeighbourPredicateType.hpp"

/**
 * @brief Distance BiPredicate Functor.
 * Emits true if the distance between the two shapes is less or equal to the maximum distance.
 * Uses WGS84Ellipsoid distance between the two closest points of the two shapes
 */
struct DistanceBiPredicate{
    double maxDistanceInMeters;

    bool operator()(fishnet::geometry::Shape auto const & lhs, fishnet::geometry::Shape auto const & rhs) const noexcept {
        auto [l,r] = fishnet::geometry::closestPoints(lhs,rhs);
        return l == r || fishnet::WGS84Ellipsoid::distance(l,r) <= maxDistanceInMeters;
    }

    static NeighbouringPredicateType type() {
        return NeighbouringPredicateType::DistanceBiPredicate;
    }
};