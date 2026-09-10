#pragma once
#include <fishnet/CollectionConcepts.hpp>
#include "GeometryBase.hpp"
#include "ISegment.hpp"

namespace fishnet::geometry::__impl{
template<typename T>
class RingStub{
public:
    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::RING;
    RingStub(std::vector<SegmentStub<T>> segments){}
    RingStub(std::vector<PointStub<T>> points){}
    bool operator==(const RingStub<T> & other) const noexcept{
        return true;
    }
    std::vector<SegmentStub<T>> getSegments() const{
        return {};
    }
    std::vector<PointStub<T>> getPoints() const{
        return {};
    }
    RingStub<T> getBoundary() const{
        return *this;
    }
    std::vector<RingStub<T>> getHoles() const{
        return {};
    }
    double area() const{
        return 0.0;
    }
    PointStub<double> centroid() const{
        return PointStub<double>(0,0);
    }
    RingStub<T> aaBB() const{
        return RingStub<T>({}, {});
    }
    bool isInside(PointStub<T> point) const{
        return true;
    }
    bool isOnBoundary(PointStub<T> point) const{
        return true;
    }
    bool isOutside(PointStub<T> point) const{
        return true;
    }
    bool contains(PointStub<T> point) const{
        return true;
    }
    bool contains(SegmentStub<T> segment) const{
        return true;
    }
    bool contains(RingStub<T> ring) const{
        return true;
    }
    bool intersects(SegmentStub<T> segment) const{
        return true;
    }
    bool intersects(LineStub<T> line) const{
        return true;
    }
    std::vector<PointStub<double>> intersections(SegmentStub<T> segment) const{
        return {};
    }
    std::vector<PointStub<double>> intersections(LineStub<T> line) const{
        return {};
    }
    bool crosses(RingStub<T> ring) const{
        return true;
    }
    bool touches(RingStub<T> ring) const{
        return true;
    }
    double distance(RingStub<T> ring) const{
        return 0.0;
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
 * @brief  Interface for a ring
 * A ring is a closed loop of segments, forming a boundary of a shape.
 * @tparam R Ring implementation type
 */
template<typename R>
concept IRing = GeometryBase<R> && requires(
    const std::remove_cvref_t<R> & ring, 
    const __impl::RingStub<typename std::remove_cvref_t<R>::numeric_type> & otherRing,
    const __impl::SegmentStub<typename std::remove_cvref_t<R>::numeric_type> & segment, 
    const __impl::LineStub<typename std::remove_cvref_t<R>::numeric_type> & line, 
    const __impl::PointStub<typename std::remove_cvref_t<R>::numeric_type> & point
){
    {ring.getSegments()} -> SegmentRange;
    {ring.getPoints()} -> PointRange;
    {ring.getBoundary()} -> std::convertible_to<std::remove_cvref_t<R>>;
    {ring.getHoles()} -> std::ranges::range; // should always be empty
    {ring.area()} -> std::convertible_to<double>;
    {ring.centroid()} -> IPoint;
    {ring.aaBB()} -> std::convertible_to<std::remove_cvref_t<R>>;
    {ring.isInside(point)} -> std::same_as<bool>;
    {ring.isOnBoundary(point)} -> std::same_as<bool>;
    {ring.isOutside(point)} -> std::same_as<bool>;
    {ring.contains(point)} -> std::same_as<bool>;
    {ring.contains(segment)} -> std::same_as<bool>;
    {ring.contains(otherRing)} -> std::same_as<bool>;
    {ring.intersects(segment)} -> std::same_as<bool>;
    {ring.intersects(line)} -> std::same_as<bool>;
    {ring.intersections(segment)} -> PointRange;
    {ring.intersections(line)} -> PointRange;
    {ring.crosses(otherRing)} -> std::same_as<bool>;
    {ring.touches(otherRing)} -> std::same_as<bool>;
    {ring.distance(otherRing)} -> std::convertible_to<double>;
};

static_assert(IRing<__impl::RingStub<double>>);
static_assert(IRing<__impl::RingStub<int>>);

template<typename R>
concept RingRange = std::ranges::range<R> && IRing<std::ranges::range_value_t<R>>;   
static_assert(RingRange<std::vector<__impl::RingStub<double>>>);
} // namespace fishnet::geometry
