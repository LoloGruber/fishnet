#pragma once
#include <fishnet/Concepts.hpp>
#include <fishnet/Radians.hpp>
#include "GeometryBase.hpp"

namespace fishnet::geometry::__impl{

template<typename T>
class PointStub{
public:
    T x;
    T y;
    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::POINT;
    constexpr PointStub(T x, T y) : x(x), y(y) {}
    PointStub operator-() const noexcept{
        return PointStub(-x, -y);
    }
    PointStub operator+(const PointStub & other) const noexcept{
        return PointStub(x + other.x, y + other.y);
    }
    PointStub operator-(const PointStub & other) const noexcept{
        return PointStub(x - other.x, y - other.y);
    }
    PointStub operator*(T scalar) const noexcept{
        return PointStub(x * scalar, y * scalar);
    }
    PointStub operator/(T scalar) const noexcept{
        return PointStub(x / scalar, y / scalar);
    }
    bool operator==(const PointStub & other) const noexcept{
        return x == other.x && y == other.y;
    }
    double dot(const PointStub & other) const noexcept{
        return 0.0;
    }
    double cross(const PointStub & other) const noexcept{
        return 0.0;
    }
    bool isParallel(const PointStub & other) const noexcept{
        return false;
    }
    bool isOrthogonal(const PointStub & other) const noexcept{
        return false;
    }
    double length() const noexcept{
        return 0.0;
    }
    double distance(const PointStub & other) const noexcept{
        return 0.0;
    }
    PointStub orthogonal() const noexcept{
        return PointStub(0,0);
    }
    PointStub normalize() const noexcept{
        return PointStub(0,0);
    }
    fishnet::math::Radians angle(const PointStub & other) const noexcept{
        return fishnet::math::Radians::PI;
    }
    fishnet::math::Radians angle(const PointStub & other, fishnet::math::Radians angleRotate) const noexcept{
        return fishnet::math::Radians::PI;
    }
    size_t hash() const noexcept {
        return 0;
    }
    std::string toString() const noexcept {
        return {};
    }
};
template<fishnet::math::Number T>
constexpr auto operator*(T scalar, const PointStub<T> & point) noexcept{
    return point * scalar;
}
} // namespace fishnet::geometry::__impl

namespace fishnet::geometry{

/**
 * @brief Interface for a point
 * @tparam P Point implementation type
 * @tparam T numeric type used for computations
 */
template<typename P>
concept IPoint = GeometryBase<P> && requires(
    const typename std::remove_cvref_t<P> & constPoint, 
    const __impl::PointStub<typename std::remove_cvref_t<P>::numeric_type> & otherPoint,
    typename std::remove_cvref_t<P>::numeric_type number
){
    {constPoint.x} -> std::convertible_to<typename std::remove_cvref_t<P>::numeric_type>;
    {constPoint.y} -> std::convertible_to<typename std::remove_cvref_t<P>::numeric_type>;
    {std::remove_cvref_t<P>(number, number)} -> std::same_as<std::remove_cvref_t<P>>;
    {-constPoint} -> std::same_as<std::remove_cvref_t<P>>;
    {constPoint + constPoint} -> std::same_as<std::remove_cvref_t<P>>;
    {constPoint - constPoint} -> std::same_as<std::remove_cvref_t<P>>;
    {constPoint * number} -> std::same_as<std::remove_cvref_t<P>>;
    {number * constPoint} -> std::same_as<std::remove_cvref_t<P>>;
    {constPoint / number} -> std::convertible_to<std::remove_cvref_t<P>>;
    {constPoint.dot(otherPoint)} -> std::convertible_to<typename std::remove_cvref_t<P>::numeric_type>;
    {constPoint.cross(otherPoint)} -> std::convertible_to<typename std::remove_cvref_t<P>::numeric_type>;
    {constPoint.isParallel(otherPoint)} -> std::same_as<bool>;
    {constPoint.isOrthogonal(otherPoint)} -> std::same_as<bool>;
    {constPoint.length()} -> std::convertible_to<double>;
    {constPoint.distance(otherPoint)} -> std::convertible_to<double>;
    {constPoint.orthogonal()} -> std::same_as<std::remove_cvref_t<P>>;
    {constPoint.normalize()} -> std::convertible_to<std::remove_cvref_t<P>>;
    {constPoint.angle(otherPoint)} -> std::same_as<fishnet::math::Radians>;
    {constPoint.angle(otherPoint, fishnet::math::Radians(0.0))} -> std::same_as<fishnet::math::Radians>;
};

static_assert(IPoint<__impl::PointStub<double>>, "PointStub<double> does not satisfy IPoint concept");
static_assert(IPoint<__impl::PointStub<int>>, "PointStub<int> does not satisfy IPoint concept");

template<typename P>
concept IPointOptional = IPoint<typename std::remove_cvref_t<P>::value_type>;

template<typename R>
concept PointRange = std::ranges::range<R> && IPoint<std::ranges::range_value_t<R>>;
static_assert(PointRange<std::vector<__impl::PointStub<double>>>, "std::vector<__impl::PointStub<double>> does not satisfy PointRange concept");
} // namespace fishnet::geometry