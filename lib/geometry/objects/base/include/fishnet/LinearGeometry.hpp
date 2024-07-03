#pragma once
#include <fishnet/NumericConcepts.hpp>
#include <fishnet/Vec2D.hpp>

namespace fishnet::geometry{

/**
 * @brief Interface for a point
 * Currently is required to be the implementation of Vec2D, but can abstracted in the future
 * @tparam P IPoint implementation type
 * @tparam T numeric type used for computations
 */
template<typename P,typename T = typename std::remove_cvref_t<P>::numeric_type>
concept IPoint = std::same_as<std::remove_cvref_t<P>,Vec2D<typename std::remove_cvref_t<P>::numeric_type>>;

/**
 * @brief Interface for a line
 * A line is defined through two points p and q, defining a direction vector.
 * A line has to support queries whether a point is on / left / right of the line.
 * @tparam L ILine implementation type
 * @tparam T numeric type used for computations
 */
template<typename L , typename T = L::numeric_type>
concept ILine = fishnet::math::Number<T> && requires (const L & line, const Vec2D<T> & point){
    {line.p} -> std::convertible_to<const Vec2D<T>>;
    {line.q} -> std::convertible_to<const Vec2D<T>>;
    {line.direction()} -> IPoint<T>;
    {line.contains(point)}-> std::same_as<bool>;
    {line.isLeft(point)} -> std::same_as<bool>;
    {line.isRight(point)} -> std::same_as<bool>;
    typename L::numeric_type;
};

namespace __impl{
/**
 * @brief Helper Interface to constrain all linear features to have at least the following operations:
 * - direction of the linear feature
 * - test if a point is on the linear feature
 * - conversion method to construct a line from any linear feature
 * @tparam L linear feature type
 * @tparam T numeric type used for computations
 */
template<typename L, typename  T=L::numeric_type>
concept requiredLinearFeatureInterface = fishnet::math::Number<T> && !ILine<L,T> && requires(const L & constLinearGeometry, const L & constOther, const Vec2D<T> & p){
    {constLinearGeometry.direction()} -> IPoint<T>;
    {constLinearGeometry.contains(p)} -> std::same_as<bool>;
    {constLinearGeometry.toLine()} -> ILine<T>;
    typename L::numeric_type;
};
};

/**
 * @brief Interface for a segment
 * A segment is formed between two points p and q, containing only the points on the line between the points
 * @tparam S segment implementation type
 * @tparam T numeric type used for computations
 */
template<typename S, typename T = S::numeric_type>
concept ISegment = __impl::requiredLinearFeatureInterface<S,T> && requires (const S & segment, const Vec2D<T> & point){
        {segment.p()} -> IPoint;
        {segment.q()} -> IPoint;
        {segment.flip()} -> std::same_as<S>; // returnes a new segment with p and q swapped
        {segment.length()} -> std::floating_point;
        {segment.upperEndpoint()} -> IPoint; // returns endpoint with greater y coordinate
        {segment.lowerEndpoint()} -> IPoint; // returns endpoint with lower y coordinate
        {segment.leftEndpoint()} -> IPoint; // returns endpoint with greater y coordinate
        {segment.rightEndpoint()} -> IPoint; // returns endpoint with lower y coordinate
        {segment.isEndpoint(point)} -> std::same_as<bool>; // test whether any the point is either p or q
        {segment.isValid()} -> std::same_as<bool>; // a segment is valid if its length is greater than 0, i.e. if p != q
        {segment.hasOverlay(segment)} -> std::same_as<bool>; // test whether the segment has a true overlay with another segment (not only touching the endpoints)
        {segment.containsSegment(segment)} -> std::same_as<bool>; // test whether the segment fully contains another segment
        {segment.touches(segment)} -> std::same_as<bool>; // test whether two segments touch each other at the endpoints
        {segment.distance(point)} -> std::floating_point; // returns minimum distance from the segment to a point
        {segment.distance(segment)} -> std::floating_point; // returns minimum distance from the segment to a point
};

/**
 * @brief Interface for a ray
 * A ray is definied through its origin and a direction.
 * @tparam R ray implementation type
 * @tparam T numeric type used for computations
 */
template<typename R, typename T = R::numeric_type>
concept IRay = __impl::requiredLinearFeatureInterface<R,T> && requires(const R & ray, const Vec2D<T> & point){
    {ray.origin()} -> IPoint;
    {ray.oppositeRay()} -> std::same_as<R>; // returns new ray with the direction vector flipped
};

/**
 * @brief Abstract Interface for any linear geometry
 * A linear geometry is either a line, ray or segment
 * @tparam L linear feature type
 * @tparam T numeric type used for computations
 */
template<typename L, typename  T=L::numeric_type>
concept LinearGeometry = fishnet::math::Number<T> && (ILine<L,T> || ISegment<L,T> || IRay<L,T> );
}

