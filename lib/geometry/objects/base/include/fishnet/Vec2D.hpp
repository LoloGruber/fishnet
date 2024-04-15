#pragma once

#include <fishnet/NumericConcepts.hpp>
#include <fishnet/Constants.hpp>
#include <fishnet/Degrees.hpp>
#include <fishnet/CantorPairing.hpp>
#include "GeometryType.hpp"

namespace fishnet::geometry{
#include <fishnet/Printable.hpp>
/**
 * Util class for geometric computations in the two-dimensional space
 */
template<fishnet::math::Number T=fishnet::math::DEFAULT_NUMERIC>
class Vec2D {
public:   
    T x;
    T y;

    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::POINT;

    template<fishnet::math::Number U, fishnet::math::Number V>
    constexpr static auto construct(U x, V y) noexcept{
        if constexpr(std::is_same_v<U,V>){
            return Vec2D<U>(x,y);
        }
        if constexpr(std::integral<U> && std::integral<V>){
            if constexpr(sizeof(U) > sizeof(V)){
                return Vec2D<U>(x,y);
            }else{
                return Vec2D<V>(x,y);
            }
        }else {
            return Vec2D<fishnet::math::DEFAULT_NUMERIC>(x,y);
        }
    }

    constexpr Vec2D(T x, T  y):x(x),y(y){
        if (fishnet::math::isZero(x)) this->x= 0.0;
        if (fishnet::math::isZero(y)) this->y= 0.0;
    }

    template<fishnet::math::Number U, fishnet::math::Number R, typename = std::enable_if_t<(!std::is_same_v<U,T> || ! std::is_same_v<R,T>)&& std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>>
    constexpr Vec2D(U x, R y):Vec2D(static_cast<T>(x),static_cast<T>(y)){}

    // template<fishnet::math::Number U, typename = std::enable_if_t<!std::is_same_v<U,T> && std::is_same_v<T,double>>>
    // constexpr Vec2D(U x, T y):Vec2D(static_cast<T>(x),y){}

    constexpr Vec2D():x(0),y(0){}

    template<fishnet::math::Number U>
    constexpr operator Vec2D<U> () const noexcept{
        return Vec2D<U>(static_cast<U>(x),static_cast<U>(y));
    }

    constexpr Vec2D<T> operator - () const {
        return Vec2D<T>(-x,-y);
    }

    template<fishnet::math::Number U>
    constexpr auto operator+(const Vec2D<U> &other) const noexcept{
        return construct(x + other.x, y + other.y);
    }

    template<fishnet::math::Number U>
    constexpr auto operator-(const Vec2D<U> &other) const noexcept{
        return construct(x-other.x, y-other.y);
    }

    template<fishnet::math::Number U>
    constexpr auto operator*(U scalar) const noexcept{
        return construct(x*scalar,y*scalar);
    }

    template<fishnet::math::Number U>
    constexpr auto operator/(U scalar) const noexcept{
        if(scalar==0) scalar=1;
        return construct(x/scalar,y/scalar);
    }

    template<fishnet::math::Number U>
    constexpr bool operator==(const Vec2D<U> & other) const noexcept{
        return fishnet::math::areEqual(x,other.x) and fishnet::math::areEqual(y,other.y);
    }

    template<fishnet::math::Number U>
    constexpr auto dot(const Vec2D<U> & other) const noexcept{
        return x * other.x + y * other.y;
    }

    template<fishnet::math::Number U>
    constexpr auto cross(const Vec2D<U> & other) const noexcept{
        return x * other.y - y * other.x;
    }

    template<fishnet::math::Number U>
    constexpr bool isParallel(const Vec2D<U> & other) const noexcept{
        return fishnet::math::isZero(cross(other));
    }

    template<fishnet::math::Number U>
    constexpr bool isOrthogonal(const Vec2D<U> & other) const noexcept{
        return fishnet::math::isZero(dot(other));
    }

    constexpr fishnet::math::DEFAULT_FLOATING_POINT length() const{
        return sqrt(dot(*this));
    }

    template<fishnet::math::Number U>
    constexpr fishnet::math::DEFAULT_FLOATING_POINT distance(const Vec2D<U> & other) const {
        return (*this-other).length();
    }

    constexpr Vec2D<T> orthogonal() const noexcept{
        return {y,-x};
    }

    constexpr auto normalize() const{
        return *this / length();
    }

    /**
     * Computes the angle (counterclockwise) for the direction vector with reference to the reference vector, starting rotation on x-axis
     * @param reference for the angle
     * @return
     */
    fishnet::math::Radians angle(const Vec2D<T> & reference) const{
        auto dir = *this - reference;
        return fishnet::math::Radians::atan2(dir.y,dir.x);
    }

    /**
     * Calculates the angle with the previous method (starting from x-axis and performing a counterclockwise rotation)
     * then rotates the angle by angleRotate
     * @param reference
     * @param angleRotate
     * @return
     */
    fishnet::math::Radians angle(const Vec2D<T> & reference, fishnet::math::Radians angleRotate) const{
        return angle(reference) + angleRotate;
    }

    constexpr std::string toString() const noexcept{
        return "(" + std::to_string(x) + "," + std::to_string(y) + ")";
    }

};
template<fishnet::math::Number T, fishnet::math::Number U>
constexpr auto operator*(U scalar,Vec2D<T> vector)  noexcept{
    return vector * scalar;
}



struct LexicographicOrder{
    template<fishnet::math::Number T,fishnet::math::Number U>
    constexpr bool operator()(const Vec2D<T> & lhs, const Vec2D<U> & rhs)const noexcept {
        if(fishnet::math::areEqual(lhs.x,rhs.x)) return lhs.y < rhs.y;
        return lhs.x < rhs.x;
    }
};

using Vec2DStd = Vec2D<fishnet::math::DEFAULT_NUMERIC>;
using Vec2DReal =Vec2D<fishnet::math::DEFAULT_FLOATING_POINT>;
}

namespace std{
    template<typename T>
    struct hash<fishnet::geometry::Vec2D<T>>{
        constexpr static auto hasher = hash<fishnet::math::DEFAULT_NUMERIC>{}; //convert all to double to keep hash consistent with equality function
        size_t operator()(const fishnet::geometry::Vec2D<T> & vector) const {
            size_t xHash = hasher(vector.x);
            size_t yHash = hasher(vector.y);
            return util::CantorPairing(xHash,yHash);
        }
    };
}

