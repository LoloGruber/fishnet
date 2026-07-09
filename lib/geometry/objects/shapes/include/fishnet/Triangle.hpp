#pragma once

#include <string>

#include <fishnet/Vec2D.hpp>
#include <fishnet/NumericConcepts.hpp>
#include <fishnet/Printable.hpp>
#include <fishnet/GeometryType.hpp>
#include <fishnet/ShapeGeometry.hpp>
#include "Ring.hpp"
#include "SimplePolygon.hpp"

namespace fishnet::geometry {

/**
 * @brief Triangle shape implementation
 * Inherits from SimplePolygon<T> (a 3-vertex ring)
 * @tparam T numeric type used for computations
 */
template<fishnet::math::Number T>
class Triangle : public SimplePolygon<T> {
public:
    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::POLYGON;

    /**
     * @brief Construct a Triangle from three vertices
     * @param a first vertex
     * @param b second vertex
     * @param c third vertex
     */
    Triangle(const Vec2D<T> & a, const Vec2D<T> & b, const Vec2D<T> & c)
        : SimplePolygon<T>(Ring<T>({a, b, c})) {}

    /**
     * @brief Get the three vertices as a random access range
     */
    constexpr util::view_of<Vec2D<T>> auto getPoints() const noexcept {
        return this->getBoundary().getPoints();
    }

    /**
     * @brief Get the three edges as a range of segments
     */
    constexpr util::view_of<Segment<T>> auto getSegments() const noexcept {
        return this->getBoundary().getSegments();
    }

    constexpr std::string toString() const noexcept {
        auto pts = getPoints();
        return "Triangle{" + pts[0].toString() + ", " + pts[1].toString() + ", " + pts[2].toString() + "}";
    }
};

static_assert(IRing<Triangle<double>>);
static_assert(Shape<Triangle<double>>);

// Deduction guide
template<math::Number T>
Triangle(const Vec2D<T> &, const Vec2D<T> &, const Vec2D<T> &) -> Triangle<T>;

// Explicit template instantiation
template class Triangle<fishnet::math::DEFAULT_NUMERIC>;

} // namespace fishnet::geometry

namespace std {
    template<typename T>
    struct hash<fishnet::geometry::Triangle<T>> {
        constexpr static auto ringHasher = hash<fishnet::geometry::Ring<T>>{};
        size_t operator()(const fishnet::geometry::Triangle<T> & triangle) const noexcept {
            return ringHasher(triangle.getBoundary()) + 1;
        }
    };
}
