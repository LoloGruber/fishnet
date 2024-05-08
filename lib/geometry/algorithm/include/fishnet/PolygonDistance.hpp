#pragma once
#include <fishnet/Vec2D.hpp>
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/FunctionalConcepts.hpp>

namespace fishnet::geometry {

namespace __impl{

static Vec2DReal closestPointOnSegment(ISegment auto const & segment, IPoint auto const & point) noexcept {
        auto orthogonal = segment.direction().orthogonal();
        auto line = Line(point,point+orthogonal);
        auto intersection = segment.intersection(line);
        if(intersection)
            return intersection.value(); // return intersection with orthogonal line trough point if present
        return segment.p().distance(point) < segment.q().distance(point)? segment.p():segment.q(); // return endpoint of segment closest to point
}

static std::pair<Vec2DReal,Vec2DReal> closestPointsBruteForce(IRing auto const & lhs,IRing auto const & rhs) noexcept {
    fishnet::math::DEFAULT_FLOATING_POINT minDistance = std::numeric_limits<fishnet::math::DEFAULT_FLOATING_POINT>::max();
    Vec2DReal bestLeft;
    Vec2DReal bestRight;
    for(const auto & s : lhs.getSegments()){
        for(const auto & p : rhs.getPoints()){
            auto pointOnSegment = closestPointOnSegment(s,p);
            if(pointOnSegment.distance(p) < minDistance){
                bestLeft = pointOnSegment;
                bestRight = p;
                minDistance = pointOnSegment.distance(p);
            }
        }
    }
    for(const auto & s : rhs.getSegments()){
        for(const auto & p: lhs.getPoints()){
            auto pointOnSegment = closestPointOnSegment(s,p);
            if(pointOnSegment.distance(p) < minDistance){
                bestLeft = p;
                bestRight = pointOnSegment;
                minDistance = pointOnSegment.distance(p);
            }
        }
    }
    return std::make_pair(bestLeft,bestRight);
}

struct DefaultDistanceFunction{
    template<typename T,typename U>
    static inline fishnet::math::DEFAULT_FLOATING_POINT operator()(const Vec2D<T> & lhs, const Vec2D<U> & rhs) noexcept {
        return lhs.distance(rhs);
    }
};

}

template<ShapeGeometry Shape, ShapeGeometry OtherShape>
static std::pair<Vec2DReal,Vec2DReal> closestPoints(const Shape & lhs, const OtherShape & rhs) noexcept {
    return __impl::closestPointsBruteForce(lhs.getBoundary(),rhs.getBoundary());
}

static fishnet::math::DEFAULT_FLOATING_POINT shapeDistance(ShapeGeometry auto const & lhs, ShapeGeometry auto const & rhs,util::BiFunction<Vec2DReal,Vec2DReal,fishnet::math::DEFAULT_FLOATING_POINT> auto const & distanceFunction) noexcept {
    const auto & [l,r] = closestPoints(lhs,rhs);
    return distanceFunction(l,r);
}

static fishnet::math::DEFAULT_FLOATING_POINT shapeDistance(ShapeGeometry auto const & lhs, ShapeGeometry auto const & rhs) noexcept {
    return shapeDistance(lhs,rhs,__impl::DefaultDistanceFunction());
}
}