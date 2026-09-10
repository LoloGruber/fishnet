#pragma once
#include "GeometryBase.hpp"
#include "IPoint.hpp"
#include <fishnet/Option.hpp>

namespace fishnet::geometry::__impl{
template<typename T>
class LineStub{
public:
    using numeric_type = T;
    constexpr static GeometryType type = GeometryType::LINE;
    PointStub<T> p() const{
        return PointStub<T>(0,0);
    }
    PointStub<T> q() const{
        return PointStub<T>(1,1);
    }
    LineStub(PointStub<T> p, PointStub<T> q){}
    LineStub(T slope, T yIntercept){}
    bool operator==(const LineStub & other) const noexcept{
        return true;
    }
    PointStub<T> direction() const{
        return PointStub<T>(1,1);
    }
    bool isVertical()const{
        return false;
    }
    bool isHorizontal()const{
        return false;
    }
    double slope()const{
        return 1.0;
    }
    fishnet::Option<double> yIntercept()const{
        return fishnet::Option<double>(0.0);    
    }
    bool contains(IPoint auto point)const{
        return true;
    }
    bool isLeft(IPoint auto point)const{
        return true;
    }
    bool isRight(IPoint auto point)const{
        return true;
    }
    bool isParallel (LineStub line)const{
        return true;
    }
    bool intersects(LineStub line)const{
        return true;
    }
    fishnet::Option<PointStub<double>> intersection(LineStub<T> line)const{
        return fishnet::Option<PointStub<double>>(PointStub<double>(0,0));
    }   
    LineStub toLine()const{
        return *this;
    }
    size_t hash() const noexcept {
        return 0;
    }
    std::string toString() const {
        return {};   
    }
};
} // namespace fishnet::geometry::__impl

namespace fishnet::geometry{

/**
 * @brief Interface for a line
 * A line is defined through two points p and q, defining a direction vector.
 * @tparam L Line implementation type
 */
template<typename L>
concept ILine = GeometryBase<L> && requires (
    const std::remove_cvref_t<L> & line, 
    const __impl::LineStub<typename std::remove_cvref_t<L>::numeric_type> & otherLine,
    const __impl::PointStub<typename std::remove_cvref_t<L>::numeric_type> & point, 
    typename std::remove_cvref_t<L>::numeric_type number
){
    {line.p()} -> IPoint;
    {line.q()} -> IPoint;
    {std::remove_cvref_t<L>(number, number)} -> std::same_as<std::remove_cvref_t<L>>; // slope and y intercept constructor
    {line.direction()} -> IPoint;
    {line.isVertical()} -> std::same_as<bool>;
    {line.isHorizontal()} -> std::same_as<bool>;
    {line.slope()} -> std::convertible_to<double>;
    {line.yIntercept()} -> std::same_as<fishnet::Option<double>>;
    {line.contains(point)}-> std::same_as<bool>;
    {line.isLeft(point)} -> std::same_as<bool>;
    {line.isRight(point)} -> std::same_as<bool>;
    {line.isParallel(otherLine)} -> std::same_as<bool>;
    {line.intersects(otherLine)} ->std::same_as<bool>;
    {line.intersection(otherLine)} -> IPointOptional;
    {line.toLine()} -> std::same_as<std::remove_cvref_t<L>>;
};
static_assert(ILine<__impl::LineStub<double>>);
static_assert(ILine<__impl::LineStub<int>>);
} // namespace fishnet::geometry

