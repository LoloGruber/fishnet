#pragma once
#include "SimplePolygon.hpp"
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

    Rectangle(Vec2D<T> const& topLeft, Vec2D<T> const& botRight):Ring<T>({{topLeft.x,topLeft.y},{botRight.x,topLeft.y},{botRight.x,botRight.y},{topLeft.x,botRight.y}}),_left(topLeft.x),_top(topLeft.y),_right(botRight.x),_bottom(botRight.y){
        if(topLeft == botRight)
            throw fishnet::geometry::InvalidGeometryException("TopLeft and BottomRight point of Rectangle are coinciding");
    }

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
}


namespace std{
    template<typename T>
    struct hash<fishnet::geometry::Rectangle<T>>{
        constexpr static auto simplePolygonHasher = hash<fishnet::geometry::Ring<T>>{};
        size_t operator()(const fishnet::geometry::Rectangle<T> & rectangle) const noexcept {
            return simplePolygonHasher(static_cast<const fishnet::geometry::Ring<T> &>(rectangle));
        }
    };
}