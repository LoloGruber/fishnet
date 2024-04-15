#pragma once
#include <fishnet/NumericConcepts.hpp>
#include <fishnet/Vec2D.hpp>

namespace fishnet::geometry{

template<typename P>
concept IPoint = std::same_as<P,Vec2D<typename P::numeric_type>>;

template<typename L , typename T = L::numeric_type>
concept ILine = fishnet::math::Number<T> && requires (const L & line, const Vec2D<T> & point){
    {line.p} -> std::convertible_to<const Vec2D<T>>;
    {line.q} -> std::convertible_to<const Vec2D<T>>;
    {line.direction()} -> std::same_as<Vec2D<T>>;
    {line.contains(point)}-> std::same_as<bool>;
    {line.isLeft(point)} -> std::same_as<bool>;
    {line.isRight(point)} -> std::same_as<bool>;
    typename L::numeric_type;
};

namespace __impl{
template<typename L, typename  T=L::numeric_type>
concept OtherLinearFeatures = fishnet::math::Number<T> && !ILine<L,T> && requires(const L & constLinearGeometry, const L & constOther, const Vec2D<T> & p){
    {constLinearGeometry.direction()} -> std::same_as<Vec2D<T>>;
    {constLinearGeometry.contains(p)} -> std::same_as<bool>;
    {constLinearGeometry.toLine()} -> ILine<T>;
    typename L::numeric_type;
};

};

template<typename S, typename T = S::numeric_type>
concept ISegment = __impl::OtherLinearFeatures<S,T> && requires (const S & segment, const Vec2D<T> & point){
        {segment.p()} -> IPoint;
        {segment.q()} -> IPoint;
        {segment.flip()} -> std::same_as<S>;
        {segment.length()} -> std::floating_point;
        {segment.upperEndpoint()} -> IPoint;
        {segment.lowerEndpoint()} -> IPoint;
        {segment.isEndpoint(point)} -> std::same_as<bool>;
        {segment.isValid()} -> std::same_as<bool>;
        {segment.hasOverlay(segment)} -> std::same_as<bool>;
        {segment.containsSegment(segment)} -> std::same_as<bool>;
        {segment.touches(segment)} -> std::same_as<bool>;
        {segment.distance(point)} -> std::floating_point;
};

template<typename R, typename T = R::numeric_type>
concept IRay = __impl::OtherLinearFeatures<R,T> && requires(const R & ray, const Vec2D<T> & point){
    {ray.origin()} -> IPoint;
    {ray.oppositeRay()} -> std::same_as<R>;
};



template<typename L, typename  T=L::numeric_type>
concept LinearGeometry = fishnet::math::Number<T> && (ILine<L,T> || ISegment<L,T> || IRay<L,T> );


}

