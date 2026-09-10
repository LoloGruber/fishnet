#pragma once
#include <fishnet/IGeometry.hpp>
#include "Ring.hpp"

namespace fishnet::geometry {
/**
 * @brief Simple polygon implementation with no holes
 * 
 * @tparam T numeric type used for computations
 */
template<fishnet::math::Number T>
class SimplePolygon : public Ring<T>{
public:
    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::POLYGON;

    using Ring<T>::contains;
    using Ring<T>::crosses;
    using Ring<T>::touches;
    using Ring<T>::distance;

    SimplePolygon(const Ring<T> & boundary):Ring<T>(boundary){}

    SimplePolygon(std::initializer_list<Vec2D<T>> && points):Ring<T>(std::move(points)){}

    SimplePolygon(util::random_access_range_of<Vec2D<T>> auto const & points):Ring<T>(points) {}

    SimplePolygon(util::random_access_range_of<Segment<T>> auto const & segments):Ring<T>(segments) {}

    constexpr util::view_of<Segment<T>> auto getSegments() = delete;

    constexpr util::random_access_range_of<Vec2D<T>> auto getPoints() = delete;

    constexpr const Ring<T> & getBoundary() const noexcept {
        return static_cast<const Ring<T> &>(*this);
    }

    constexpr util::view_of<Ring<T>> auto getHoles() const noexcept {
        return std::ranges::empty_view<Ring<T>>();
    }

    constexpr Ring<T> aaBB() const noexcept {
        return this->getBoundary().aaBB();
    }

    template<fishnet::math::Number U>
    constexpr bool inline operator==(const SimplePolygon<U> & other) const noexcept {
        return this->getBoundary() == other.getBoundary();
    }

    constexpr bool isSimple() const noexcept {
        return true;
    }

    constexpr bool inline containsInHole(IPoint auto const & point) const noexcept {
        return false;
    }

    constexpr bool inline containsInHole(IPolygon auto const & other) const noexcept {
        return false;
    }

    constexpr bool contains(IPolygon auto const & other) const noexcept {
        return this->getBoundary().contains(other.getBoundary());
    }

    constexpr bool crosses(IPolygon auto const & other) const noexcept {
        return this->getBoundary().crosses(other.getBoundary());
    }

    constexpr bool touches(IPolygon auto const & other) const noexcept {
        return this->getBoundary().touches(other.getBoundary());
    }

    constexpr fishnet::math::DEFAULT_FLOATING_POINT distance(IPolygon auto const & other) const noexcept {
        if(this->contains(other)) return -1;
        if(this->touches(other)) return 0;
        return this->getBoundary().distance(other.getBoundary());
    }

};

//Deduction guides
template<math::Number T>
SimplePolygon(const Ring<T> &)->SimplePolygon<T>;

template<typename T>
SimplePolygon(std::initializer_list<Vec2D<T>> && points)->SimplePolygon<T>;

template<std::ranges::random_access_range R>
SimplePolygon(const R & )->SimplePolygon<typename std::ranges::range_value_t<R>::numeric_type>;

} // namespace fishnet::geometry

namespace std{
    template<typename T>
    struct hash<fishnet::geometry::SimplePolygon<T>>{
        constexpr static auto ringHasher = hash<fishnet::geometry::Ring<T>>{};
        size_t operator()(const fishnet::geometry::SimplePolygon<T> & polygon) const noexcept {
            return ringHasher(polygon.getBoundary()) +1;
        }
    };
}

namespace fishnet::geometry{
static_assert(Shape<SimplePolygon<double>>);
static_assert(IPolygon<SimplePolygon<double>>);
// Explicit template instantiation
template class SimplePolygon<fishnet::math::DEFAULT_NUMERIC>;
} // namespace fishnet::geometry