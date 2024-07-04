#pragma once

#include <fishnet/NumericConcepts.hpp>
#include <fishnet/Constants.hpp>
#include <fishnet/Degrees.hpp>
#include <fishnet/CantorPairing.hpp>
#include "GeometryType.hpp"
#include <fishnet/Printable.hpp>
namespace fishnet::geometry{

/**
 * @brief Implementation of a two-dimensional vector
 * 
 * @tparam T numeric type used for computations
 */
template<fishnet::math::Number T=fishnet::math::DEFAULT_NUMERIC>
class Vec2D {
public:   
    T x;
    T y;

    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::POINT;

    /**
     * @brief Helper factory method to construct the correct type of vector depending on the numeric types
     * 
     * @tparam U numeric type for x
     * @tparam V numeric type for y
     * @param x 
     * @param y 
     * @return vector of a common numeric type that avoid narrowing conversions
     */
    template<fishnet::math::Number U, fishnet::math::Number V>
    constexpr static auto construct(U x, V y) noexcept{
        if constexpr(std::is_same_v<U,V>){
            return Vec2D<U>(x,y);
        }
        if constexpr(std::integral<U> && std::integral<V>){
            if constexpr(sizeof(U) > sizeof(V)){ // use bigger integral type (U=long, V=int -> Vec2D<long>) 
                return Vec2D<U>(x,y);
            }else{
                return Vec2D<V>(x,y);
            }
        }else {
            return Vec2D<fishnet::math::DEFAULT_NUMERIC>(x,y); // use default numeric type in any other case
        }
    }

    constexpr Vec2D(T x, T  y):x(x),y(y){
        if (fishnet::math::isZero(x)) // utilize helper function to round floating point values very close to zero.
             this->x= 0.0;
        if (fishnet::math::isZero(y))
             this->y= 0.0;
    }

    /**
     * @brief Constructor for heterogenous types
     * Converts to the default numeric type
     * @tparam U numeric type of x
     * @tparam R numeric type of y
     * @tparam typename _ enable if -> only allow if T is the default numeric type and if either U or R is not the same type as T to prevent ambiguities
     * @tparam std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>> 
     */
    template<fishnet::math::Number U, fishnet::math::Number R, typename = std::enable_if_t<(!std::is_same_v<U,T> || ! std::is_same_v<R,T>)&& std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>>
    constexpr Vec2D(U x, R y):Vec2D(static_cast<T>(x),static_cast<T>(y)){}

    /**
     * @brief Default construction to (0,0)
     * 
     */
    constexpr Vec2D():x(0),y(0){}

    /**
     * @brief Cast operator from Vec2D<T> to Vec2D<U>
     * 
     * @tparam U numeric type of resulting vector
     * @return Vec2D<U> 
     */
    template<fishnet::math::Number U>
    constexpr operator Vec2D<U> () const noexcept{
        return Vec2D<U>(static_cast<U>(x),static_cast<U>(y));
    }

    /**
     * @brief Negation operator
     * e.g.: -Vec(2,1) => Vec(-2,-1)
     * @return constexpr Vec2D<T> 
     */
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
        if(scalar==0) 
            scalar=1; // prevent division by zero, return the same Vec2D instead
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
     * Computes the angle (counterclockwise) for the direction vector with reference at reference point, starting rotation on x-axis
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
/**
 * @brief Reversed multiplication operator, allowing commutative behavior:
 * e.g.: Vec2D(1,1) * 2 == 2 * Vec2D(1,1)
 * @tparam T numeric type of the Vec2D
 * @tparam U numeric type of scalar
 * @param scalar 
 * @param vector 
 * @return returns the result with swapped order, utilizing the implementation within Vec2D
 */
template<fishnet::math::Number T, fishnet::math::Number U>
constexpr auto operator*(U scalar,Vec2D<T> vector)  noexcept{
    return vector * scalar;
}

/**
 * @brief Comparator for lexigraphically-ordering of Vec2D objects
 * Compares first the x-Coordinate and then the y-Coordinate
 */
struct LexicographicOrder{
    template<fishnet::math::Number T,fishnet::math::Number U>
    constexpr bool operator()(const Vec2D<T> & lhs, const Vec2D<U> & rhs)const noexcept {
        if(fishnet::math::areEqual(lhs.x,rhs.x))
             return lhs.y < rhs.y;
        return lhs.x < rhs.x;
    }
};

/**
 * @brief Comparator for lexigraphically-ordering of Vec2D objects, with y Coordinate first
 */
struct YLexicographicOrder{
    template<fishnet::math::Number T,fishnet::math::Number U>
    constexpr inline bool operator()(const Vec2D<T> & lhs, const Vec2D<U> & rhs)const noexcept {
        if(fishnet::math::areEqual(lhs.y,rhs.y))
             return lhs.x < rhs.x;
        return lhs.y < rhs.y;
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
            return fishnet::util::CantorPairing(xHash,yHash); // utilize cantor pairing to keep hashes unique: hash(Vec2D(2,1)) != hash(Vec2D(1,2))
        }
    };
}

