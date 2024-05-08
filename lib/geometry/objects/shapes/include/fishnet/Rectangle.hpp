#pragma once
#include "SimplePolygon.hpp"
#include <iostream>
namespace fishnet::geometry {


template<fishnet::math::Number T>
class Rectangle:public Ring<T>{
private:
    T _left;
    T _right;
    T _top;
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
    Rectangle(util::random_access_range_of<Vec2D<T>> auto const& points):Ring<T>(points){
        init();
    }

    Rectangle(util::random_access_range_of<Segment<T>> auto const& segments):Ring<T>(segments){
        init();
    }

    Rectangle(std::initializer_list<Vec2D<T>> && points):Ring<T>(std::move(points)){
        init();
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
        return Rectangle<T>({
            {left,top}, {right,top},{right,bottom},{left,bottom}
        });
    }

};
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