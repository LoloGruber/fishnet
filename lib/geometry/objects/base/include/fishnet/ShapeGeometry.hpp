#pragma once
#include <fishnet/CollectionConcepts.hpp>
#include <fishnet/Segment.hpp>
#include <fishnet/NumericConcepts.hpp>
#include <fishnet/Printable.hpp>
namespace fishnet::geometry{

namespace __impl{

/**
 * @brief Helper Interface for defining common queries on shapes
 * 
 * @tparam S shape implementation type
 * @tparam std::remove_cvref_t<S>::numeric_type 
 */
template<typename S, typename T = typename std::remove_cvref_t<S>::numeric_type>
concept LocationQueries = fishnet::math::Number<T> && requires(const S & shape, const Vec2D<T> & point , const Segment<T> & segment, const Line<T> & line){
    {shape.contains(point)} -> std::same_as<bool>;
    {shape.contains(segment)} -> std::same_as<bool>;
    {shape.isInside(point)} -> std::same_as<bool>;
    {shape.isOnBoundary(point)} -> std::same_as<bool>;
    {shape.isOutside(point)} -> std::same_as<bool>;
    {shape.intersects(segment)} -> std::same_as<bool>;
    {shape.intersects(line)} -> std::same_as<bool>;
    {shape.intersections(segment)} -> fishnet::util::forward_range_of<Vec2D<double>>;
    {shape.intersections(line)} -> fishnet::util::forward_range_of<Vec2D<double>>;
};

/**
 * @brief Helper Interface for defining common queries between shapes
 * 
 * @tparam S 
 * @tparam O 
 */
template<typename S, typename O>
concept ShapeQueries = requires (const S & shape, const O & other){
    {shape.contains(other)} -> std::same_as<bool>;
    {shape.crosses(other)} -> std::same_as<bool>;
    {shape.touches(other)} -> std::same_as<bool>;
    {shape.distance(other)} -> std::same_as<math::DEFAULT_FLOATING_POINT>;
};

/**
 * @brief Helper Interface for polygons with holes
 * 
 * @tparam S 
 * @tparam O 
 */
template<typename S,typename O>
concept HolesQueries = requires(const S & shape, const O & other) {
    {shape.isInHole(other)} -> std::same_as<bool>;
};
}

#include "LinearGeometry.hpp"

template<typename R>
concept SegmentRange = std::ranges::range<R> && ISegment<std::ranges::range_value_t<R>>;

template<typename V>
concept SegmentView = SegmentRange<V> && std::ranges::viewable_range<V>;

/**
 * @brief Interface for a ring
 * Requires location queries and access to the segments / points forming the ring
 * @tparam R ring implementation type
 * @tparam std::remove_cvref_t<R>::numeric_type numeric type used for computations
 */
template<typename R, typename T=typename std::remove_cvref_t<R>::numeric_type>
concept IRing =  fishnet::math::Number<T> && __impl::LocationQueries<R> && requires (const R & ring){
        typename std::remove_cvref_t<R>::numeric_type;
        {ring.getSegments()} -> SegmentView;
        {ring.getPoints()} -> util::view_of<Vec2D<T>>;

};

template<typename R>
concept RingRange = std::ranges::range<R> && IRing<std::ranges::range_value_t<R>>;

/**
 * @brief Interface for a polygon
 * A polygon consists of a boundary and zero or more holes fully contained inside the boundary
 * @tparam P 
 * @tparam std::remove_cvref_t<P>::numeric_type 
 */
template<typename P, typename T = typename std::remove_cvref_t<P>::numeric_type>
concept IPolygon = fishnet::math::Number<T> && __impl::LocationQueries<P> && not IRing<P> && requires (const P & polygon){
    {polygon.getBoundary()} -> IRing<T>; 
    {polygon.getHoles()} -> RingRange;
    {polygon.getHoles()} -> std::ranges::viewable_range; // getHoles() has to return a view on the rings defining the holes.
};

template<typename R>
concept PolygonRange = std::ranges::range<R> && IPolygon<std::ranges::range_value_t<R>>;

/**
 * @brief Interface for a multi-polygon
 * A multi-polygon is a collection of polygons that do not overlap each other
 * @tparam M 
 */
template<typename M>
concept IMultiPolygon = __impl::LocationQueries<M> && requires(const M & multiPolygon){
    typename std::remove_cvref_t<M>::polygon_type;
    typename std::remove_cvref_t<M>::numeric_type;
    {multiPolygon.getPolygons()} -> PolygonRange;
    {multiPolygon.getPolygons()} -> std::ranges::viewable_range; // getPolygons() returns a view on the polygons stored in the multi-polygon
};

/**
 * @brief Interface for any Shape
 * A Shape is either a ring, a polygon or multi-polygon. In addition to the interface for the geometry type, the following helper interfaces must be fullfilled:
 * Ring: ShapeQueries
 * Polygon: ShapeQueries + HolesQueries
 * MultiPolygon: ShapeQueries + HolesQueries on the polygon type
 * Additionally, for every shape methods for the area, axis-aligned bounding box and the centroid point are required
 * @tparam S shape type
 * @tparam S::numeric_type used for computations
 */
template<typename S, typename T =typename std::remove_cvref_t<S>::numeric_type>
concept Shape = ((IRing<S,T> or (IPolygon<S,T> and  __impl::HolesQueries<S,S> )) and __impl::ShapeQueries<S,S>) or (IMultiPolygon<S> and __impl::ShapeQueries<S,typename S::polygon_type> and __impl::HolesQueries<S,typename S::polygon_type>) 
    && requires(const S & shape){
        {shape.area()} -> std::same_as<math::DEFAULT_FLOATING_POINT>;
        {shape.centroid()} -> std::same_as<Vec2D<math::DEFAULT_FLOATING_POINT>>;
        {shape.aaBB()} -> IRing<T>;
    };
}