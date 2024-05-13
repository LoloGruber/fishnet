#pragma once
#include <optional>
#include <fishnet/Vec2D.hpp>
#include "Line.hpp"

namespace fishnet::geometry{
/**
 * @brief Implementation of a segment
 * 
 * @tparam T numeric type used for computations
 */
template<fishnet::math::Number T = fishnet::math::DEFAULT_NUMERIC>
class Segment{
private:
    Vec2D<T> _p;
    Vec2D<T> _q;
public:

    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::SEGMENT;

    constexpr Segment(Vec2D<T> p, Vec2D<T> q):_p(p),_q (q){}

    template<fishnet::math::Number U, typename = std::enable_if_t<!std::is_same_v<U,T> && std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>>
    constexpr Segment(Vec2D<T> p, Vec2D<U> q):Segment(p,Vec2D<T> (q.x ,q.y)){}

    template<fishnet::math::Number U, typename = std::enable_if_t<!std::is_same_v<U,T> && std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>>
    constexpr Segment(Vec2D<U> p, Vec2D<T> q):Segment(Vec2D<T>(p.x ,p.y) ,q){}

    constexpr Vec2D<T> p() const noexcept {
        return _p;
    }

    constexpr Vec2D<T> q() const noexcept {
        return _q;
    }

    constexpr Segment<T> flip() const noexcept {
        return Segment(this->_q,this->_p);
    }

    constexpr Vec2D<T> direction() const noexcept {
        return _q-_p;
    }

    constexpr Vec2D<T> upperEndpoint() const noexcept {
        if (fishnet::math::areEqual(_p.y,_q.y)) return _p.x < _q.x ? _p:_q;
        return _p.y > _q.y ? _p:_q;
    }

    constexpr Vec2D<T> lowerEndpoint() const noexcept {
        if(fishnet::math::areEqual(_p.y,_q.y)) return _p.x < _q.x ? _q:_p;
        return _p.y > _q.y ? _q:_p;
    }

    constexpr bool isEndpoint(IPoint auto const & point) const noexcept {
        return _p == point or _q == point;
    }

    constexpr fishnet::math::DEFAULT_FLOATING_POINT length() const noexcept {
        return direction().length();
    }

    constexpr bool isValid() const noexcept{
        return length() > 0;
    }

    constexpr Line<T> toLine() const noexcept {
        return Line<T>(_p,_q);
    }

    constexpr bool isParallel(LinearGeometry auto const& other)const noexcept{
        return areParallel(*this,other);
    }

    constexpr bool intersects(LinearGeometry auto const& other)const noexcept{
        return intersect(*this,other);
    }

    /**
     * @brief Two Segments have an Overlay if the are contained on the same line: 
     *  -> the Segments must be parallel and at least share on point.
     * 
     * @tparam U 
     * @param other 
     * @return true 
     * @return false 
     */
    template<fishnet::math::Number U>
    constexpr bool hasOverlay(const Segment<U> & other) const noexcept{
        return isParallel(other) and (contains(other.p()) or contains(other.q()));
    }

    template<fishnet::math::Number U>
    constexpr bool containsSegment(const Segment<U> & other) const noexcept {
        return isParallel(other) and contains(other.p()) and contains(other.q());
    }

    /**
     * @brief Two Segments touch each other when one endpoint is equal and
     * neither Segment is fully contained in the other one
     * 
     * @tparam U 
     * @param other 
     * @return true 
     * @return false 
     */
    template<fishnet::math::Number U>
    constexpr bool touches(const Segment<U> & other) const noexcept{
        return (this->_p == other.p() or this->_p == other.q() or this->_q == other.q() or this->_q == other.p())
            && not containsSegment(other)
            && not other.containsSegment(*this);
    }

    template<fishnet::math::Number U>
    constexpr bool operator==(const Segment<U> & other) const noexcept {
        return (this->_p == other.p() and this->_q == other.q()) or (this->_q==other.p() and this->_p == other.q());
    }

    constexpr bool contains(IPoint auto const & point)const noexcept{
        using FLOAT_TYPE = fishnet::math::DEFAULT_FLOATING_POINT;
        auto EPSILON = fishnet::math::EPSILON;
        if (not isValid()) // 0-length segment
            return _p==point; 
        auto dir = direction();
        if(dir.x == 0){ // vertical segment
            FLOAT_TYPE lambdaY = FLOAT_TYPE(point.y - _p.y) / FLOAT_TYPE(dir.y); // lambda has to be between in the range [0,1] and the x-Coordinates have to be approx. equal
            return lambdaY >= 0.0-EPSILON and lambdaY <= 1.0 + EPSILON and fabs(_p.x - point.x) < EPSILON;
        }
        if(dir.y == 0){ // horizontal segment
            FLOAT_TYPE lambdaX = FLOAT_TYPE(point.x - _p.x) / FLOAT_TYPE(dir.x); // lambda has to be between in the range [0,1] and the y-Coordinates have to be approx. equal
            return lambdaX >= 0.0 - EPSILON and lambdaX <= 1.0 + EPSILON and fabs(_p.y - point.y) < EPSILON;
        }
        FLOAT_TYPE lambdaX = FLOAT_TYPE(point.x - _p.x) / FLOAT_TYPE(dir.x);
        FLOAT_TYPE lambdaY = FLOAT_TYPE(point.y - _p.y) / FLOAT_TYPE(dir.y);
        return fabs(lambdaX - lambdaY) < EPSILON and lambdaX >= 0.0 - EPSILON and lambdaY <= 1.0 + EPSILON; // both lambdas have to be equal by definition, so checking one for the range [0,1] is sufficient 
    }

    constexpr fishnet::math::DEFAULT_FLOATING_POINT distance(IPoint auto const & point) const noexcept {
        auto orthogonalDirection = this->direction().orthogonal();
        auto orthogonalLine = Line(point, point + orthogonalDirection);
        auto intersection = this->intersection(orthogonalLine);
        if(intersection) 
            return point.distance(*intersection); // intersection on the segment of orthogonal line between segment and point
        return std::min(_p.distance(point), _q.distance(point)); // return closest endpoint of segment to the point otherwise
    }

    constexpr auto intersection(LinearGeometry auto const& other)const noexcept {
        return linearIntersection(*this,other);
    }

    constexpr std::string toString() const noexcept {
        return "[" + this->_p.toString() +","+this->_q.toString()+"]";
    }
};
static_assert(LinearGeometry<Segment<double>>);
static_assert(ISegment<Segment<double>>);
}
namespace std{
    template<typename T>
    struct hash<fishnet::geometry::Segment<T>>{
        constexpr static auto hasher = hash<fishnet::geometry::Vec2D<T>>{};
        size_t operator()(const fishnet::geometry::Segment<T> & segment) const {
            return hasher(segment.p()) ^ hasher(segment.q());
        }
    };
}