#pragma once
#include "DistanceFunction.hpp"


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
};

template<fishnet::util::BiFunction<fishnet::geometry::Vec2DReal,fishnet::geometry::Vec2DReal, fishnet::math::DEFAULT_FLOATING_POINT> DistanceFunction, fishnet::geometry::Shape S>
struct DistancePredicate{
    DistanceFunction distanceFunction;
    S referenceShape;
    double maxDistanceInMeters;
    
    bool operator()(fishnet::geometry::Shape auto const & shape) const noexcept {
        auto [l,r] = fishnet::geometry::closestPoints(referenceShape,shape);
        return l == r || distanceFunction(l,r) <= maxDistanceInMeters;
    }      
};

