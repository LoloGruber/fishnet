#pragma once
#include <optional>

#include "Vec2D.hpp"
#include "Line.hpp"


namespace fishnet::geometry{
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

    // template<fishnet::math::Number U>
    // constexpr bool intersects(const Segment<U> & other) const noexcept {
    //     [[likely]] if(this->isValid() && other.isValid()) return intersect(*this,other);
    //     if(this->isValid()) return this->contains(other.p());
    //     return other.contains(this->_p);
    // }

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
        using namespace fishnet::math;
        if (not isValid()) return _p==point;
        auto dir = direction();
        if(dir.x == 0){
            DEFAULT_FLOATING_POINT lambdaY = DEFAULT_FLOATING_POINT(point.y - _p.y) / DEFAULT_FLOATING_POINT(dir.y);
            return lambdaY >= 0.0-EPSILON and lambdaY <= 1.0 + EPSILON and fabs(_p.x - point.x) < EPSILON;
        }
        if(dir.y == 0){
            DEFAULT_FLOATING_POINT lambdaX = DEFAULT_FLOATING_POINT(point.x - _p.x) / DEFAULT_FLOATING_POINT(dir.x);
            return lambdaX >= 0.0 - EPSILON and lambdaX <= 1.0 + EPSILON and fabs(_p.y - point.y) < EPSILON;
        }
        DEFAULT_FLOATING_POINT lambdaX = DEFAULT_FLOATING_POINT(point.x - _p.x) / DEFAULT_FLOATING_POINT(dir.x);
        DEFAULT_FLOATING_POINT lambdaY = DEFAULT_FLOATING_POINT(point.y - _p.y) / DEFAULT_FLOATING_POINT(dir.y);
        return fabs(lambdaX - lambdaY) < EPSILON and lambdaX >= 0.0 - EPSILON and lambdaY <= 1.0 + EPSILON; // both lambdas have to be equal by definition, so checking one is sufficient
    }

    constexpr fishnet::math::DEFAULT_FLOATING_POINT distance(IPoint auto const & point) const noexcept {
        auto orthogonalDirection = this->direction().orthogonal();
        auto orthogonalLine = Line(point, point + orthogonalDirection);
        auto intersection = this->intersection(orthogonalLine);
        if(intersection) return point.distance(*intersection);
        return std::min(_p.distance(point), _q.distance(point));
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