#pragma once
#include "IPolygon.hpp"

namespace fishnet::geometry::__impl{
template<typename T>
class MultiPolygonStub{
public:
    using numeric_type = T;
    using polygon_type = PolygonStub<T>;
    constexpr static GeometryType type = GeometryType::MULTIPOLYGON;
    MultiPolygonStub(std::vector<PolygonStub<T>> polygons){}
    MultiPolygonStub(PolygonStub<T> polygon){}
    bool operator==(const MultiPolygonStub<T> & other) const noexcept{
        return true;
    }
    std::vector<PolygonStub<T>> getPolygons() const{
        return {};
    }
    bool addPolygon(PolygonStub<T> polygon){
        return true;
    }
    bool removePolygon(PolygonStub<T> polygon){
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
    bool containsInHole(PolygonStub<T> polygon) const{
        return true;
    }
    bool contains(PointStub<T> point) const{
        return true;
    }   
    bool contains(SegmentStub<T> segment) const{
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
    std::vector<PointStub<T>> intersections(SegmentStub<T> segment) const{
        return {};      
    }
    std::vector<PointStub<T>> intersections(LineStub<T> line) const{
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
    size_t hash() const noexcept {
        return 0;
    }
    std::string toString() const {
        return {};
    }
};
} // namespace fishnet::geometry::__impl

namespace fishnet::geometry{

/**
 * @brief Interface for a multi-polygon
 * A multi-polygon is a collection of polygons.
 * @tparam M MultiPolygon implementation type
 */
template<typename M>
concept IMultiPolygon = GeometryBase<M> && requires(
    const std::remove_cvref_t<M> & multiPolygon,
    std::remove_cvref_t<M> & mutableMultiPolygon,
    const typename std::remove_cvref_t<M>::polygon_type & ownPolygon,
    const __impl::PolygonStub<typename std::remove_cvref_t<M>::numeric_type> & polygon,
    const __impl::PointStub<typename std::remove_cvref_t<M>::numeric_type> & point,
    const __impl::SegmentStub<typename std::remove_cvref_t<M>::numeric_type> & segment,
    const __impl::LineStub<typename std::remove_cvref_t<M>::numeric_type> & line
){
    typename std::remove_cvref_t<M>::polygon_type;
    {multiPolygon.getPolygons()} -> PolygonRange;
    {mutableMultiPolygon.addPolygon(ownPolygon)} -> std::same_as<bool>;
    {mutableMultiPolygon.removePolygon(ownPolygon)} -> std::same_as<bool>;
    {multiPolygon.area()} -> std::convertible_to<double>;
    {multiPolygon.centroid()} -> IPoint;
    {multiPolygon.aaBB()} -> IRing;
    {multiPolygon.isInside(point)} -> std::same_as<bool>;
    {multiPolygon.isOnBoundary(point)} -> std::same_as<bool>;
    {multiPolygon.isOutside(point)} -> std::same_as<bool>;
    {multiPolygon.containsInHole(polygon)} -> std::same_as<bool>;
    {multiPolygon.contains(point)} -> std::same_as<bool>;
    {multiPolygon.contains(segment)} -> std::same_as<bool>;
    {multiPolygon.contains(polygon)} -> std::same_as<bool>;
    {multiPolygon.intersects(segment)} -> std::same_as<bool>;
    {multiPolygon.intersects(line)} -> std::same_as<bool>;
    {multiPolygon.intersections(segment)} -> PointRange;
    {multiPolygon.intersections(line)} -> PointRange;
    {multiPolygon.crosses(polygon)} -> std::same_as<bool>;
    {multiPolygon.touches(polygon)} -> std::same_as<bool>;
    {multiPolygon.distance(polygon)} -> std::convertible_to<double>;
};

static_assert(IMultiPolygon<__impl::MultiPolygonStub<double>>);
static_assert(IMultiPolygon<__impl::MultiPolygonStub<int>>);
}
