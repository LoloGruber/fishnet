#pragma once
#include <fishnet/IGeometry.hpp>
#include <algorithm>
#include "Ring.hpp"
#include <iostream>

namespace fishnet::geometry {

/**
 * @brief Implementation of a rectangle
 * Stores top,right,bottom and left point of the rectangle
 * @tparam T numeric type used for computations
 */
template<fishnet::math::Number T>
class Rectangle:public Ring<T>{
private:
    T _left;
    T _top;
    T _right;
    T _bottom;

    void init() noexcept {
        for(const auto p : this->getPoints() | std::views::take(1)){ 
            _left = p.x;
            _right = _left;
            _top =  p.y;
            _bottom = _top;
        }
        for(const auto p : this->getPoints()){
            if(p.y > _top)
                _top = p.y;
            if(p.y < _bottom)
                _bottom = p.y;
            if(p.x > _right)
                _right = p.x;
            if(p.x < _left)
                _left = p.x;
        }
    }

public:
    Rectangle(Shape auto const & ring):Ring<T>(ring.aaBB().getPoints()){
        init();
    }

    Rectangle(T left, T top, T right, T bottom):Ring<T>({{left,top},{right,top},{right,bottom},{left,bottom}}),_left(left),_top(top),_right(right),_bottom(bottom){}

    /**
     * @brief Construct the rectangle spanned by two opposite corners
     * @note the corners may be given in any order, they are sorted into left/top/right/bottom
     */
    Rectangle(Vec2D<T> const& corner, Vec2D<T> const& oppositeCorner)
        :Rectangle(std::min(corner.x, oppositeCorner.x), std::max(corner.y, oppositeCorner.y),
                   std::max(corner.x, oppositeCorner.x), std::min(corner.y, oppositeCorner.y)) {}

    T left() const noexcept {
        return _left;
    }

    T right() const noexcept {
        return _right;
    }

    T top() const noexcept {
        return _top;
    }

    T bottom() const noexcept {
        return _bottom;
    }

    const Rectangle<T> & aaBB() const noexcept {
        return *this;
    }

    Rectangle<T> scale(T factor) const noexcept {
        if (factor <= 0){
            std::cerr << "Scale factor must be greater than 0!" << std::endl;
            return Rectangle<T>(*this);
        }
        auto xDiff = (_right - _left) * (factor-1);
        auto yDiff = (_top - _bottom) * (factor-1);
        auto top = _top + yDiff;
        auto bottom = _bottom - yDiff;
        auto left = _left - xDiff;
        auto right = _right + xDiff;
        return Rectangle<T>(left,top,right,bottom);
    }

    template<fishnet::math::Number U>
    bool overlap(const Rectangle<U> & other) const noexcept {
        if(this->right() < other.left() || this->left() > other.right())
            return false;
        if(this->top() < other.bottom() || this->bottom() > other.top())
            return false;
        return true;
    }

};

//Deduction guide:
template<Shape S>
Rectangle(const S &) -> Rectangle<typename S::numeric_type>;

// Explicit template instantiation
template class Rectangle<fishnet::math::DEFAULT_NUMERIC>;

/**
 * @brief Helper function to get the common bounding box for a range of polygons
 * 
 * @tparam R type of polygon range
 * @param polygons polygons to compute the bounding box around
 * @return Rectangle<typename std::ranges::range_value_t<R>::numeric_type> BoundingBox using the numeric type like the input
 */
template<std::ranges::range R>
static Rectangle<typename std::ranges::range_value_t<R>::numeric_type> minimalBoundingBox(const R & polygons) {
    static_assert(Shape<std::ranges::range_value_t<R>>, "Range value type must be a Shape");
    using number = typename std::ranges::range_value_t<R>::numeric_type;
    number left = std::numeric_limits<number>::max();
    number right = std::numeric_limits<number>::lowest();
    number top = std::numeric_limits<number>::lowest();
    number bottom = std::numeric_limits<number>::max();
    if(util::isEmpty(polygons)){
        throw std::runtime_error("Range of polygons is empty, cannot compute bounding box");
    }
    for(const auto & polygon: polygons){
        auto aaBB = Rectangle<number>(polygon);
        if(aaBB.left() < left)
            left = aaBB.left();
        if(aaBB.right() > right)
            right = aaBB.right();
        if(aaBB.top() > top)
            top = aaBB.top();
        if(aaBB.bottom() < bottom)
            bottom = aaBB.bottom();
    }
    return Rectangle<number>(left,top,right,bottom);
}
} // namespace fishnet::geometry


namespace std{
    template<typename T>
    struct hash<fishnet::geometry::Rectangle<T>>{
        constexpr static auto simplePolygonHasher = hash<fishnet::geometry::Ring<T>>{};
        size_t operator()(const fishnet::geometry::Rectangle<T> & rectangle) const noexcept {
            return simplePolygonHasher(static_cast<const fishnet::geometry::Ring<T> &>(rectangle));
        }
    };
}

namespace fishnet::geometry{
static_assert(IEnvelope<Rectangle<double>>);
static_assert(Shape<Ring<double>>);
static_assert(IRing<Ring<double>>);
static_assert(IRing<Rectangle<double>>);
} // namespace fishnet::geometry
