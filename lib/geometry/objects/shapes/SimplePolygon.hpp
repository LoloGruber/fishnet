#pragma once
#include "ShapeGeometry.hpp"
#include "LinearGeometry.hpp"
#include "Ring.hpp"

namespace fishnet::geometry {

template<fishnet::math::Number T>
class SimplePolygon : public Ring<T>{
public:
    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::POLYGON;

    SimplePolygon(const Ring<T> & boundary):Ring<T>(boundary){}

    SimplePolygon(std::initializer_list<Vec2D<T>> && points):Ring<T>(std::move(points)){}

    SimplePolygon(util::random_access_range_of<Vec2D<T>> auto const & points):Ring<T>(points) {}

    SimplePolygon(util::random_access_range_of<Segment<T>> auto const & segments):Ring<T>(segments) {}


    constexpr const Ring<T> & getBoundary() const noexcept {
        return static_cast<const Ring<T> &>(*this);
    }

    constexpr util::view_of<Ring<T>> auto getHoles() const noexcept {
        return std::ranges::empty_view<Ring<T>>();
    }

    constexpr SimplePolygon<T> aaBB() const noexcept {
        return SimplePolygon<T>(this->getBoundary().aaBB());
    }

    template<fishnet::math::Number U>
    constexpr bool inline operator==(const SimplePolygon<U> & other) const noexcept {
        return this->getBoundary() == other.getBoundary();
    }

    constexpr bool inline isInHole(IPolygon auto const & other ) const noexcept {
        return false;
    }

};
static_assert(IPolygon<SimplePolygon<double>>);
static_assert(ShapeGeometry<SimplePolygon<double>>);
}
namespace std{
    template<typename T>
    struct hash<fishnet::geometry::SimplePolygon<T>>{
        constexpr static auto ringHasher = hash<fishnet::geometry::Ring<T>>{};
        size_t operator()(const fishnet::geometry::SimplePolygon<T> & polygon) const noexcept {
            return ringHasher(polygon.getBoundary()) +1;
        }
    };
}