#pragma once
#include <fishnet/Vec2D.hpp>
#include "Line.hpp"

namespace fishnet::geometry{

/**
 * @brief Implementation of a ray
 * A ray consist of an origin and the direction of the ray
 * @tparam T numeric type used for computations
 */
template<fishnet::math::Number T>
class Ray{
private:
    const Vec2D<T> originPoint; 
    const Vec2D<T> directionVector;
public: 
    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::RAY;

    constexpr static Ray<T> up(Vec2D<T> origin) noexcept {
        return Ray<T>(origin, Vec2D<T>(0,1));
    }

    constexpr static Ray<T> down(Vec2D<T> origin) noexcept {
        return Ray<T>(origin, Vec2D<T>(0,-1));
    }

    constexpr static Ray<T> right(Vec2D<T> origin) noexcept {
        return Ray<T>(origin,Vec2D<T>(1,0));
    }

    constexpr static Ray<T> left(Vec2D<T> origin) noexcept {
        return Ray<T>(origin,Vec2D<T>(-1,0));
    }

    constexpr Ray(Vec2D<T> origin, Vec2D<T> direction): originPoint(origin), directionVector(direction){
        [[unlikely]] if (direction == Vec2D(0,0))
             throw std::invalid_argument("Zero Vector not valid for the direction of a Ray");
    }

    template<fishnet::math::Number U, typename = std::enable_if_t<!std::is_same_v<U,T> && std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>>
    constexpr Ray(Vec2D<T> origin, Vec2D<U> direction): Ray(origin,static_cast<Vec2D<T>>(direction)){}

    template<fishnet::math::Number U, typename =std::enable_if_t<!std::is_same_v<U,T> && std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>>
    constexpr Ray(Vec2D<U> origin ,Vec2D<T> direction): Ray(static_cast<Vec2D<T>>(origin),direction){}

    constexpr Vec2D<T> direction() const noexcept {
        return this->directionVector;
    }    

    constexpr Vec2D<T> origin() const noexcept {
        return this->originPoint;
    }

    constexpr Line<T> toLine() const noexcept {
        return Line<T>(originPoint,originPoint+directionVector);
    }

    constexpr Ray<T> oppositeRay() const noexcept{
        return Ray(originPoint, -directionVector);
    }

    constexpr Ray<T> operator - () const {
        return oppositeRay();
    }

    constexpr bool isParallel(LinearGeometry auto const& other) const noexcept{
        return areParallel(*this,other);
    }

    constexpr bool intersects(LinearGeometry auto const& other) const noexcept {
        return intersect(*this,other);
    }

    constexpr bool contains(IPoint auto const & point) const noexcept{
        using FLOAT_TYPE = fishnet::math::DEFAULT_FLOATING_POINT;
        if(point==originPoint) 
            return true;
        if(directionVector.x == 0){ // vertical ray -> point is on ray if its on the origin or above/below depending on the direction vector
            return point.x == originPoint.x and (directionVector.y > 0 ? point.y > originPoint.y : point.y < originPoint.y);
        }
        if(directionVector.y == 0){ // horizontal ray -> point is on ray if its on the origin or left/right depending on the direction vector
            return point.y == originPoint.y and (directionVector.x > 0 ? point.x > originPoint.x : point.x < originPoint.x);
        }
        FLOAT_TYPE lX = FLOAT_TYPE(point.x - originPoint.x) / FLOAT_TYPE(directionVector.x);
        FLOAT_TYPE lY = FLOAT_TYPE(point.y - originPoint.y) / FLOAT_TYPE(directionVector.y);
        return fishnet::math::areEqual(lX,lY) and (lX > 0); // lambda values have to equal and positive to be on the correct side of the direction vector
    }

    template<fishnet::math::Number U> 
    constexpr bool operator==(const Ray<U> & other) const noexcept {
        return this->originPoint == other.origin() and this->direction().normalize() == other.direction().normalize();
    }

    constexpr std::optional<Vec2DReal> intersection(LinearGeometry auto const& other) const noexcept {
        return linearIntersection(*this,other);
    }

    constexpr std::string toString() const {
        return "Ray "+this->originPoint.toString()+" + k * "+this->directionVector.toString();
    }
};
static_assert(LinearGeometry<Ray<double>>);
}
namespace std{
    template<typename T>
    struct hash<fishnet::geometry::Ray<T>>{
        constexpr static auto hasher = hash<fishnet::geometry::Vec2D<T>>{};
        constexpr static auto doubleHasher = hash<fishnet::geometry::Vec2D<fishnet::math::DEFAULT_NUMERIC>>{};
        size_t operator()(const fishnet::geometry::Ray<T> & ray) const {
            size_t origin_hash = hasher(ray.origin());
            size_t direction_hash = doubleHasher(ray.direction().normalize());
            return fishnet::util::CantorPairing(origin_hash,direction_hash);
        }
    };
}
