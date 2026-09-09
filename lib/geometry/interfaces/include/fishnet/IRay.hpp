#pragma once
#include "GeometryBase.hpp"
#include "IPoint.hpp"
#include "ILine.hpp"

namespace fishnet::geometry::__impl{
template<typename T>
class RayStub{
public:
    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::RAY;
    PointStub<T> origin() const{
        return PointStub<T>(0,0);
    }
    PointStub<T> direction() const{
        return PointStub<T>(1,1);
    }
    RayStub(PointStub<T> origin, PointStub<T> direction){}
    bool operator==(const RayStub & other) const noexcept{
        return true;
    }
    RayStub<T> oppositeRay() const{
        return RayStub(origin(), -direction());
    }
    RayStub<T> operator-() const{
        return oppositeRay();
    }
    bool isParallel(LineStub<T> line) const{
        return true;
    }
    bool isParallel(RayStub<T> ray) const{
        return true;
    }
    bool intersects(LineStub<T> line) const{
        return true;
    }
    bool intersects(RayStub<T> ray) const{
        return true;
    }
    bool contains(PointStub<T> point) const{
        return true;
    }
    fishnet::Option<PointStub<double>> intersection(LineStub<T> line) const{
        return {};
    }
    fishnet::Option<PointStub<double>> intersection(RayStub ray) const{
        return {};
    }
    LineStub<T> toLine() const{
        return LineStub(origin(), origin()+direction());
    }
    std::string toString() const{
        return {};
    }
};
} // namespace fishnet::geometry::__impl

namespace std{
    template<typename T>
    struct hash<fishnet::geometry::__impl::RayStub<T>>{
        constexpr static auto hasher = hash<fishnet::math::DEFAULT_NUMERIC>{}; //convert all to double to keep hash consistent with equality function
        size_t operator()(const fishnet::geometry::__impl::RayStub<T> & ray) const noexcept{
            return 0;
        }
    };
};

namespace fishnet::geometry{

/**
 * @brief Interface for a ray
 * A ray is defined through its origin and a direction.
 * @tparam R Ray implementation type
 */
template<typename R>
concept IRay = GeometryBase<R> && requires(
    const std::remove_cvref_t<R> & ray, 
    const __impl::RayStub<typename std::remove_cvref_t<R>::numeric_type> & otherRay,
    const __impl::PointStub<typename std::remove_cvref_t<R>::numeric_type> & point, 
    const __impl::LineStub<typename std::remove_cvref_t<R>::numeric_type> & line
){
    {ray.origin()} -> IPoint;
    {ray.direction()} -> IPoint;
    {ray.oppositeRay()} -> std::same_as<std::remove_cvref_t<R>>; // returns new ray with the direction vector flipped
    {-ray} -> std::same_as<std::remove_cvref_t<R>>; // returns new ray with the direction vector flipped
    {ray.isParallel(line)} -> std::same_as<bool>;
    {ray.isParallel(otherRay)} -> std::same_as<bool>;
    {ray.intersects(line)} -> std::same_as<bool>;
    {ray.intersects(otherRay)} -> std::same_as<bool>;
    {ray.contains(point)} -> std::same_as<bool>;
    {ray.intersection(line)} -> IPointOptional;
    {ray.intersection(otherRay)} -> IPointOptional;
    {ray.toLine()} -> ILine;
};

static_assert(IRay<__impl::RayStub<double>>);
static_assert(IRay<__impl::RayStub<int>>);
} // namespace fishnet::geometry