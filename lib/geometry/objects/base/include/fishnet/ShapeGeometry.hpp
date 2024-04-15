#pragma once
#include <fishnet/CollectionConcepts.hpp>
#include <fishnet/Segment.hpp>
#include <fishnet/NumericConcepts.hpp>
namespace fishnet::geometry{
#include <fishnet/Printable.hpp>
namespace __impl{
template<typename S, typename T = typename std::remove_cvref_t<S>::numeric_type>
concept LocationQueries = fishnet::math::Number<T> && requires(const S & shape, const Vec2D<T> & p , const Segment<T> & s, const Line<T> & l){
    {shape.contains(p)} -> std::same_as<bool>;
    {shape.contains(s)} -> std::same_as<bool>;
    {shape.isInside(p)} -> std::same_as<bool>;
    {shape.isOnBoundary(p)} -> std::same_as<bool>;
    {shape.isOutside(p)} -> std::same_as<bool>;
    {shape.intersects(s)} -> std::same_as<bool>;
    {shape.intersects(l)} -> std::same_as<bool>;
    {shape.intersections(s)} -> util::forward_range_of<Vec2D<double>>;
    {shape.intersections(l)} -> util::forward_range_of<Vec2D<double>>;
};

template<typename S, typename O>
concept ShapeQueries = requires (const S & shape, const O & other){
    {shape.contains(other)} -> std::same_as<bool>;
    {shape.crosses(other)} -> std::same_as<bool>;
    {shape.touches(other)} -> std::same_as<bool>;
    {shape.distance(other)} -> std::same_as<math::DEFAULT_FLOATING_POINT>;
};

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

template<typename R, typename T=typename std::remove_cvref_t<R>::numeric_type>
concept IRing =  fishnet::math::Number<T> && __impl::LocationQueries<R> && requires (const R & ring, const Vec2D<T> & p, const Segment<T> & s){
        typename std::remove_cvref_t<R>::numeric_type;
        {ring.getSegments()} -> SegmentView;
        {ring.getPoints()} -> util::view_of<Vec2D<T>>;

};

template<typename R>
concept RingRange = std::ranges::range<R> && IRing<std::ranges::range_value_t<R>>;

template<typename P, typename T = typename P::numeric_type>
concept IPolygon = fishnet::math::Number<T> && __impl::LocationQueries<P> && requires (const P & polygon,const Vec2D<T> & p){
    {polygon.getBoundary()} -> IRing<T>; 
    {polygon.getHoles()} -> RingRange;
    {polygon.getHoles()} -> std::ranges::viewable_range;
};


template<typename R>
concept PolygonRange = std::ranges::range<R> && IPolygon<std::ranges::range_value_t<R>>;

template<typename M>
concept IMultiPolygon = __impl::LocationQueries<M> && requires(const M & multiPolygon){
    typename std::remove_cvref_t<M>::polygon_type;
    typename std::remove_cvref_t<M>::numeric_type;
    {multiPolygon.getPolygons()} -> PolygonRange;
    {multiPolygon.getPolygons()} -> std::ranges::viewable_range;
};

template<typename S, typename T =typename S::numeric_type>
concept ShapeGeometry = ((IRing<S,T> or (IPolygon<S,T> and  __impl::HolesQueries<S,S> )) and __impl::ShapeQueries<S,S>) or (IMultiPolygon<S> and __impl::ShapeQueries<S,typename S::polygon_type> and __impl::HolesQueries<S,typename S::polygon_type>) 
    && requires(const S & shape, const Vec2D<T> & p ){
        {shape.area()} -> std::same_as<math::DEFAULT_FLOATING_POINT>;
        {shape.centroid()} -> std::same_as<Vec2D<math::DEFAULT_FLOATING_POINT>>;
        {shape.aaBB()} -> IRing<T>;
    };
}