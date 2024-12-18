#pragma once
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/WGS84Ellipsoid.hpp>
#include "NeighbourPredicateType.hpp"

/**
 * @brief Distance BiPredicate Functor.
 * Emits true if the distance between the two shapes is less or equal to the maximum distance (in meters).
 * @tparam DistanceFunction type determining how to calculate the distance between two points in meters.
 */
template<fishnet::util::BiFunction<fishnet::geometry::Vec2DReal,fishnet::geometry::Vec2DReal, fishnet::math::DEFAULT_FLOATING_POINT> DistanceFunction>
struct DistanceBiPredicate{
    DistanceFunction distanceFunction;
    double maxDistanceInMeters;

    bool operator()(fishnet::geometry::Shape auto const & lhs, fishnet::geometry::Shape auto const & rhs) const noexcept {
        auto [l,r] = fishnet::geometry::closestPoints(lhs,rhs);
        return l == r || distanceFunction(l,r) <= maxDistanceInMeters;
    }      

    static NeighbouringPredicateType type() {
        return NeighbouringPredicateType::DistanceBiPredicate;
    }
};

struct WGS84Distance{
    static auto operator()(const fishnet::geometry::Vec2DReal & lhs, const fishnet::geometry::Vec2DReal & rhs) noexcept {
        return fishnet::WGS84Ellipsoid::distance(lhs,rhs);
    }
};

struct MetricDistance{
    static auto operator()(const fishnet::geometry::Vec2DReal & lhs, const fishnet::geometry::Vec2DReal & rhs) noexcept {
        return lhs.distance(rhs);
    }
};

using DistanceFunction = std::function<fishnet::math::DEFAULT_FLOATING_POINT(const fishnet::geometry::Vec2DReal &, const fishnet::geometry::Vec2DReal &)>;

static DistanceFunction distanceFunctionForSpatialReference(const OGRSpatialReference & spatialRef) {
    if(spatialRef.IsEmpty())
        throw std::runtime_error("No distance function available for empty coordinate system");
    if(spatialRef.IsLocal() || spatialRef.IsProjected())
        return MetricDistance();
    if(spatialRef.IsGeographic())
        return WGS84Distance();
    throw std::runtime_error("No distance function found for coordinate system: \""+std::string(spatialRef.GetName())+"\"");
}