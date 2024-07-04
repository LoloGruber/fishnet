#pragma once
#include <fishnet/Vec2D.hpp>
#include <fishnet/ShapeGeometry.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include "SweepLine.hpp"
#include <fishnet/Segment.hpp>

namespace fishnet::geometry {

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
 * @brief Compute the pair of nearest points for two segments
 * 
 * @param lhs 
 * @param rhs 
 * @return closest pair of points
 */
static std::pair<Vec2DReal,Vec2DReal> closestPoints(ISegment auto const & lhs, ISegment auto const & rhs) noexcept {
    std::array<std::pair<Vec2DReal,Vec2DReal>,4> points {{
        {closestPointOnSegment(lhs,rhs.p()),rhs.p()},
        {closestPointOnSegment(lhs,rhs.q()),rhs.q()},
        {closestPointOnSegment(rhs,lhs.p()),lhs.p()},
        {closestPointOnSegment(rhs,lhs.q()),lhs.q()}
    }};
    return *std::ranges::min_element(points,[](const auto & left, const auto & right){
        const auto & [lFrom,lTo] = left;
        const auto & [rFrom,rTo] = right;
        return lFrom.distance(lTo) < rFrom.distance(rTo);
    });
}

namespace __impl{

class ClosestPointsResult;

class PolygonReference{
private:
    friend class ClosestPointsResult;
    enum class Type{
        THIS,OTHER,NONE
    }; 
    Type type;

    PolygonReference():type(Type::NONE){}
    PolygonReference(Type type):type(type){}

    constexpr bool inline isNone() const noexcept {
        return this->type==Type::NONE;
    }
public:
    static PolygonReference THIS() {
        return PolygonReference(Type::THIS);
    }

    static PolygonReference OTHER() {
        return PolygonReference(Type::OTHER);
    }

    constexpr bool inline operator<(const PolygonReference & other) const noexcept {
        return type < other.type;
    }

    constexpr bool inline operator==(const PolygonReference & other) const noexcept {
        return type == other.type;
    }

    PolygonReference other() const noexcept {
        if(type == Type::THIS)
            return PolygonReference::OTHER();
        else
            return PolygonReference::THIS();
    }

};

struct PolygonPoint {
    constexpr static LexicographicOrder ordering {};
    Segment<fishnet::math::DEFAULT_FLOATING_POINT> segment;
    PolygonReference polygonRef;

    PolygonPoint(Segment<fishnet::math::DEFAULT_FLOATING_POINT> && segment, PolygonReference polygonReference)
    :segment(std::move(segment)),polygonRef(polygonReference){}

    constexpr const auto & getPoint() const noexcept {
        return segment.p();
    }

    constexpr const auto & getSegment() const noexcept {
        return segment;
    }

    constexpr bool inline operator<(const PolygonPoint& other ) const noexcept {
        if(polygonRef==other.polygonRef)
            return ordering(this->getPoint(),other.getPoint());
        return polygonRef < other.polygonRef;
    }
};

class ClosestPointsResult{
private:
    bool foundPointOfOtherPolygon = false;
    PolygonReference polyRef;
public:
    Vec2DReal p1;
    Vec2DReal p2;
    fishnet::math::DEFAULT_FLOATING_POINT minDistance;

    ClosestPointsResult(fishnet::math::DEFAULT_FLOATING_POINT initialMinDistance):minDistance(initialMinDistance){}

    bool startSearch(PolygonReference polygonRef){
        if(foundPointOfOtherPolygon){
            return true;
        }
        [[unlikely]] if(polyRef.isNone()){
            polyRef = polygonRef;
        }
        if(polyRef != polygonRef){
            foundPointOfOtherPolygon = true;
        }
        return foundPointOfOtherPolygon;
    }
};

using PolygonPointSweepLine = SweepLine<PolygonPoint,ClosestPointsResult,std::less<PolygonPoint>,false,std::less<fishnet::math::DEFAULT_NUMERIC>>;

struct PolygonSegmentSweepEvent : public PolygonPointSweepLine::InsertEvent {
    PolygonSegmentSweepEvent(const PolygonPoint & point):PolygonPointSweepLine::InsertEvent(point){}
    virtual fishnet::math::DEFAULT_NUMERIC eventPoint() const noexcept {
        return this->obj->getPoint().x;
    }
    virtual void process(PolygonPointSweepLine & sweepLine, ClosestPointsResult & status) const {
        const auto & polygonPoint = *this->obj;
        const auto & point = polygonPoint.getPoint();
        const auto & segment = polygonPoint.getSegment();
        sweepLine.addSLS(this->obj);
        if(not status.startSearch(polygonPoint.polygonRef))
            return;
        auto leftQuery  = PolygonPoint({segment.leftEndpoint() - Vec2DReal(status.minDistance,0),point},polygonPoint.polygonRef.other());
        auto rightQuery = PolygonPoint({segment.rightEndpoint() + Vec2DReal(status.minDistance,0),point},polygonPoint.polygonRef.other());
        auto lowerBound = sweepLine.lowerBound(leftQuery);
        auto upperBound = sweepLine.upperBound(rightQuery);
        for(auto it= lowerBound; it != upperBound; ++it){
            const auto & currentSegment = (*it)->getSegment();
            auto [l,r] = closestPoints(segment,currentSegment);
            auto distance = l.distance(r);
            if(distance < status.minDistance){
                status.minDistance = distance;
                status.p1 = l;
                status.p2 = r;
            }
        }
    }
};

static std::pair<Vec2DReal,Vec2DReal> closestPointsSweep(IRing auto const & lhs, IRing auto const & rhs) noexcept {
    PolygonPointSweepLine sweepLine;
    ClosestPointsResult status {std::ranges::begin(lhs.getSegments())->p().distance(std::ranges::begin(rhs.getSegments())->p())};
    std::vector<PolygonPoint> segments;
    std::ranges::for_each(lhs.getSegments(),[&segments](const auto & segmentLeft){
        segments.push_back(PolygonPoint(Segment<fishnet::math::DEFAULT_FLOATING_POINT>(Vec2DReal(segmentLeft.leftEndpoint()),Vec2DReal(segmentLeft.rightEndpoint())),PolygonReference::THIS()));
        segments.push_back(PolygonPoint(Segment<fishnet::math::DEFAULT_FLOATING_POINT>(Vec2DReal(segmentLeft.rightEndpoint()),Vec2DReal(segmentLeft.leftEndpoint())),PolygonReference::THIS()));
    });
    std::ranges::for_each(rhs.getSegments(),[&segments](const auto & segmentRight){
        segments.push_back(PolygonPoint(Segment<fishnet::math::DEFAULT_FLOATING_POINT>(Vec2DReal(segmentRight.leftEndpoint()),Vec2DReal(segmentRight.rightEndpoint())),PolygonReference::OTHER()));
        segments.push_back(PolygonPoint(Segment<fishnet::math::DEFAULT_FLOATING_POINT>(Vec2DReal(segmentRight.rightEndpoint()),Vec2DReal(segmentRight.leftEndpoint())),PolygonReference::OTHER()));
    });
    std::ranges::for_each(segments,[&sweepLine](const auto & segment){
        sweepLine.addEvent(std::make_unique<PolygonSegmentSweepEvent>(segment));
    });
    auto result = sweepLine.sweep(status);
    return {result.p1,result.p2};
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
/*     if(fishnet::util::size(lhs.getSegments())+ fishnet::util::size(rhs.getSegments()) > 100){
        return __impl::closestPointsSweep(lhs.getBoundary(),rhs.getBoundary());
    } */
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