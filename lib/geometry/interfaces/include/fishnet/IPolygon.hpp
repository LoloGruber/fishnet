#pragma once
#include "GeometryBase.hpp"
#include "IRing.hpp"

namespace fishnet::geometry::__impl{
template<typename T>
class PolygonStub{
public:
    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::POLYGON;
    PolygonStub(RingStub<T> boundary, std::vector<RingStub<T>> holes){}
    bool operator==(const PolygonStub<T> & other) const noexcept{
        return true;
    }
    RingStub<T> getBoundary() const{
        return RingStub<T>({}, {});
    }
    std::vector<RingStub<T>> getHoles() const{
        return {};
    }
    bool isSimple() const{
        return true;
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
    bool isInHole(PointStub<T> point) const{
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
    bool contains(PolygonStub<T> polygon) const{
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
    bool crosses(PolygonStub<T> polygon) const{
        return true;
    }
    bool touches(PolygonStub<T> polygon) const{
        return true;
    }
    double distance(PolygonStub<T> polygon) const{
        return 0.0;
    }
    std::string toString() const{
        return {};
    }
};
} // namespace fishnet::geometry::__impl

namespace std{
    template<typename T>
    struct hash<fishnet::geometry::__impl::PolygonStub<T>>{
        std::size_t operator()(const fishnet::geometry::__impl::PolygonStub<T> & p) const noexcept{
            return 0;
        }
    };
}

namespace fishnet::geometry{

/**
 * @brief Interface for a polygon
 * A polygon is a two-dimensional shape defined by a boundary and zero or more holes.
 * @tparam P Polygon implementation type
 */
template<typename P>
concept IPolygon = GeometryBase<P> && requires(
    const std::remove_cvref_t<P> & polygon, 
    const __impl::PolygonStub<typename std::remove_cvref_t<P>::numeric_type> & otherPolygon,
    const __impl::RingStub<typename std::remove_cvref_t<P>::numeric_type> & ring, 
    const __impl::PointStub<typename std::remove_cvref_t<P>::numeric_type> & point, 
    const __impl::SegmentStub<typename std::remove_cvref_t<P>::numeric_type> & segment, 
    const __impl::LineStub<typename std::remove_cvref_t<P>::numeric_type> & line
){
    {polygon.getBoundary()} -> IRing;
    {polygon.getHoles()} -> RingRange;
    {polygon.isSimple()} -> std::same_as<bool>;
    {polygon.area()} -> std::convertible_to<double>;
    {polygon.centroid()} -> IPoint;
    {polygon.aaBB()} -> IRing;
    {polygon.isInside(point)} -> std::same_as<bool>;
    {polygon.isOnBoundary(point)} -> std::same_as<bool>;
    {polygon.isOutside(point)} -> std::same_as<bool>;
    {polygon.isInHole(point)} -> std::same_as<bool>;
    {polygon.contains(point)} -> std::same_as<bool>;
    {polygon.contains(segment)} -> std::same_as<bool>;
    {polygon.contains(ring)} -> std::same_as<bool>;
    {polygon.contains(otherPolygon)} -> std::same_as<bool>;
    {polygon.intersects(segment)} -> std::same_as<bool>;
    {polygon.intersects(line)} -> std::same_as<bool>;
    {polygon.intersections(segment)} -> PointRange;
    {polygon.intersections(line)} -> PointRange;
    {polygon.crosses(otherPolygon)} -> std::same_as<bool>;
    {polygon.touches(otherPolygon)} -> std::same_as<bool>;
    {polygon.distance(otherPolygon)} -> std::convertible_to<double>;
};

static_assert(IPolygon<__impl::PolygonStub<double>>);
static_assert(IPolygon<__impl::PolygonStub<int>>);

template<typename R>
concept PolygonRange = std::ranges::forward_range<R> && IPolygon<std::ranges::range_value_t<R>>;
static_assert(PolygonRange<std::vector<__impl::PolygonStub<double>>>);
} // namespace fishnet::geometry