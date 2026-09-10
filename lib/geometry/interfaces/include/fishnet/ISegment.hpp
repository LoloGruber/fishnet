#pragma once
#include "GeometryBase.hpp"
#include "IPoint.hpp"
#include "ILine.hpp"

namespace fishnet::geometry::__impl{
template<typename T>
class SegmentStub{
public:
    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::SEGMENT;
    PointStub<T> p() const{
        return PointStub<T>(0,0);  
    }
    PointStub<T> q() const{
        return PointStub<T>(1,1);  
    }
    SegmentStub(PointStub<T> p, PointStub<T> q){}
    bool operator==(const SegmentStub & other) const noexcept{
        return true;
    }
    PointStub<T> direction() const{
        return PointStub<T>(1,1);
    }
    double length() const{
        return 0.0;
    }
    PointStub<T> upperEndpoint() const{
        return PointStub<T>(0,0);
    }
    PointStub<T> lowerEndpoint() const{
        return PointStub<T>(0,0);
    }
    PointStub<T> leftEndpoint() const{
        return PointStub<T>(0,0);
    }
    PointStub<T> rightEndpoint() const{
        return PointStub<T>(0,0);
    }
    bool isEndpoint(PointStub<T> point) const{
        return true;
    }
    bool isValid() const{
        return true;
    }
    bool overlaps(SegmentStub segment) const{
        return true;
    }
    bool contains(PointStub<T> point) const{
        return true;
    }
    bool contains(SegmentStub segment) const{
        return true;
    }
    bool touches(SegmentStub segment) const{
        return true;
    }
    double distance(PointStub<T> point) const{
        return 0.0;
    }
    double distance(SegmentStub segment) const{
        return 0.0;
    }
    bool isParallel(LineStub<T> line) const{
        return true;
    }
    bool isParallel(SegmentStub segment) const{
        return true;
    }
    bool intersects(LineStub<T> line) const{
        return true;
    }
    bool intersects(SegmentStub segment) const{
        return true;
    }
    std::optional<PointStub<T>> intersection(LineStub<T> line) const{
        return {};
    }
    std::optional<PointStub<T>> intersection(SegmentStub segment) const{
        return {};
    }
    LineStub<T> toLine() const{
        return LineStub(p(),q());
    }
    SegmentStub flip() const{
        return SegmentStub(q(),p());
    }
    size_t hash() const noexcept {
        return 0;
    }
    std::string toString() const{
        return {};
    }
};
} // namespace fishnet::geometry::__impl

namespace fishnet::geometry{

/**
 * @brief Interface for a segment
 * A segment is formed between two points p and q, containing only the points on the line between the points
 * @tparam S segment implementation type
 */
template<typename S>
concept ISegment = GeometryBase<S> && requires (
    const std::remove_cvref_t<S> & segment,
    const __impl::SegmentStub<typename std::remove_cvref_t<S>::numeric_type> & otherSegment,
    const __impl::PointStub<typename std::remove_cvref_t<S>::numeric_type> & point, 
    const __impl::LineStub<typename std::remove_cvref_t<S>::numeric_type> & line
){
        {segment.p()} -> IPoint;
        {segment.q()} -> IPoint;
        {segment.direction()} -> IPoint;
        {segment.length()} -> std::convertible_to<double>;
        {segment.upperEndpoint()} -> IPoint; 
        {segment.lowerEndpoint()} -> IPoint; 
        {segment.leftEndpoint()} -> IPoint; 
        {segment.rightEndpoint()} -> IPoint; 
        {segment.isEndpoint(point)} -> std::same_as<bool>; // test whether any the point is either p or q
        {segment.isValid()} -> std::same_as<bool>; // a segment is valid if its length is greater than 0, i.e. if p != q
        {segment.overlaps(otherSegment)} -> std::same_as<bool>; // test whether the segment has a true overlay with another segment (not only touching the endpoints)
        {segment.contains(point)} -> std::same_as<bool>;
        {segment.contains(otherSegment)} -> std::same_as<bool>; 
        {segment.touches(otherSegment)} -> std::same_as<bool>; 
        {segment.distance(point)} -> std::convertible_to<double>; 
        {segment.distance(otherSegment)} -> std::convertible_to<double>; 
        {segment.isParallel(line)} -> std::same_as<bool>;
        {segment.isParallel(otherSegment)} -> std::same_as<bool>;
        {segment.intersects(line)} -> std::same_as<bool>;
        {segment.intersects(otherSegment)} -> std::same_as<bool>;
        {segment.intersection(line)} -> IPointOptional;
        {segment.intersection(otherSegment)} -> IPointOptional;
        {segment.toLine()} -> ILine;
        {segment.flip()} -> std::same_as<std::remove_cvref_t<S>>; // returns a new segment with p and q swapped
};

static_assert(ISegment<__impl::SegmentStub<double>>);
static_assert(ISegment<__impl::SegmentStub<int>>);

template<typename R>
concept SegmentRange = std::ranges::range<R> && ISegment<std::ranges::range_value_t<R>>;
static_assert(SegmentRange<std::vector<__impl::SegmentStub<double>>>);
} // namespace fishnet::geometry