#pragma once
#include <fishnet/Vec2D.hpp>
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/FunctionalConcepts.hpp>

namespace fishnet::geometry {

namespace __impl{

/**
 * @brief Compute the point on the segment, closest to the query point
 * 
 * @param segment 
 * @param point 
 * @return
 */
static Vec2DReal closestPointOnSegment(ISegment auto const & segment, IPoint auto const & point) noexcept {
        auto orthogonal = segment.direction().orthogonal();
        auto line = Line(point,point+orthogonal);
        auto intersection = segment.intersection(line); // intersection with the orthogonal line from the segment to the point
        if(intersection)
            return intersection.value(); // return intersection with orthogonal line trough point if present
        return segment.p().distance(point) < segment.q().distance(point)? segment.p():segment.q(); // return endpoint of segment closest to point
}

/**
 * @brief Compute the pair of closest points of two rings
 * 
 * @param lhs ring object
 * @param rhs ring object
 * @return std::pair<Vec2DReal,Vec2DReal> 
 */
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

/**
 * @brief Default distance function, which uses the distance between the vectors 
 * The result is in the same units, as the vectors
 */
struct DefaultDistanceFunction{
    template<typename T,typename U>
    static inline fishnet::math::DEFAULT_FLOATING_POINT operator()(const Vec2D<T> & lhs, const Vec2D<U> & rhs) noexcept {
        return lhs.distance(rhs);
    }
};

}

/**
 * @brief Generic Shape overload to compute the closest pair of points of two shapes
 * This function uses the solely the boundary of the shape for the closest points, assuming that the shapes do not contain each other.
 * @tparam Shape 
 * @tparam OtherShape 
 * @param lhs 
 * @param rhs 
 * @return std::pair<Vec2DReal,Vec2DReal> 
 */
template<Shape ShapeType, Shape OtherShapeType>
static std::pair<Vec2DReal,Vec2DReal> closestPoints(const ShapeType & lhs, const OtherShapeType & rhs) noexcept {
    return __impl::closestPointsBruteForce(lhs.getBoundary(),rhs.getBoundary());
}

/**
 * @brief Compute the distance between two shapes, with custom distance function
 * 
 * @param lhs 
 * @param rhs 
 * @param distanceFunction custom distance function. Allows computation in other units (e.g. meters instead of angular units)
 * @return fishnet::math::DEFAULT_FLOATING_POINT 
 */
static fishnet::math::DEFAULT_FLOATING_POINT shapeDistance(Shape auto const & lhs, Shape auto const & rhs,util::BiFunction<Vec2DReal,Vec2DReal,fishnet::math::DEFAULT_FLOATING_POINT> auto const & distanceFunction) noexcept {
    const auto & [l,r] = closestPoints(lhs,rhs);
    return distanceFunction(l,r);
}

/**
 * @brief Default shapeDistance overload
 * 
 * @param lhs 
 * @param rhs 
 * @return fishnet::math::DEFAULT_FLOATING_POINT 
 */
static fishnet::math::DEFAULT_FLOATING_POINT shapeDistance(Shape auto const & lhs, Shape auto const & rhs) noexcept {
    return shapeDistance(lhs,rhs,__impl::DefaultDistanceFunction());
}
}