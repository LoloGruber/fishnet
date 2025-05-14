#pragma once
#include <fishnet/Vec2D.hpp>
#include <optional>
#include <numeric>
#include <fishnet/LinearGeometry.hpp>
#include <fishnet/LinearIntersection.hpp>

namespace fishnet::geometry{
/**
 * @brief Implementation of a line
 * 
 * @tparam T numeric type used for computations
 */
template<fishnet::math::Number T = fishnet::math::DEFAULT_NUMERIC>
class Line{
public:
    const Vec2D<T> p;
    const Vec2D<T> q;

    constexpr static Line<T> X_AXIS = Line<T>(T(0),T(0));
    constexpr static Line<T> Y_AXIS = Line<T>(Vec2D<T>(0,0),Vec2D<T>(0,1));

    /**
     * @brief Vertical line factory 
     * 
     * @param x x-axis intersection of the vertical line
     * @return constexpr Line<T> 
     */
    constexpr static Line<T> verticalLine(T x) noexcept {
        return Line<T>(Vec2D<T>(x,0),Vec2D<T>(x,1));
    }

    /**
     * @brief Horizontal line factory
     * 
     * @param y y-axis intersection of the horizontal line
     * @return constexpr Line<T> 
     */
    constexpr static Line<T> horizontalLine(T y) noexcept {
        return Line<T>(0,y);
    }

    using numeric_type =  T;
    constexpr static GeometryType type = GeometryType::LINE;

    /**
     * @brief Constructor for a line formed by two points
     * 
     */
    constexpr Line(Vec2D<T> p , Vec2D<T> q):p(p),q(q){
        if (p == q) 
            throw std::invalid_argument("Coinciding Points cannot define a Line");
    }

    /**
     * @brief Constructor for a line with heterogenous numeric types
     * Construct new line by converting to the default numeric type
     * @tparam U numeric type != T
     * @tparam typename enable_if -> only allow this constructor if T is the numeric type:
     * @tparam std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>> 
     */
    template<fishnet::math::Number U, typename = std::enable_if_t<!std::is_same_v<U,T> && std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>>
    constexpr Line(Vec2D<T> p, Vec2D<U> q):Line(p,static_cast<Vec2D<T>>(q)){}

    /**
     * @brief Constructor for a line with heterogenous numeric types
     * Construct new line by converting to the default numeric type
     * @tparam U numeric type != T
     * @tparam typename enable_if -> only allow this constructor if T is the numeric type:
     * @tparam std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>> 
     */
    template<fishnet::math::Number U, typename = std::enable_if_t<!std::is_same_v<U,T> && std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>>
    constexpr Line(Vec2D<U> p , Vec2D<T> q):Line(static_cast<Vec2D<T>>(p),q){}

    /**
     * @brief Constructor of line using slope and y intercept
     * 
     */
    constexpr Line(T slope, T yIntercept):p(Vec2D<T>(0,yIntercept)),q(Vec2D<T>(1,slope+yIntercept)){}

    /**
     * @brief Constructor of line using slope and y intercept with heterogenous type
     * 
     * @tparam U numeric type != T
     * @tparam typename enable_if -> only allow this constructor if T is the numeric type:
     * @tparam std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>
     */
    template<fishnet::math::Number U, typename = std::enable_if_t<!std::is_same_v<U,T> && std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>>
    constexpr Line(T slope, U yIntercept):Line(slope,fishnet::math::DEFAULT_NUMERIC(yIntercept)){}

    /**
     * @brief Constructor of line using slope and y intercept with heterogenous type
     * 
     * @tparam U numeric type != T
     * @tparam typename enable_if -> only allow this constructor if T is the numeric type:
     * @tparam std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>
     */
    template<fishnet::math::Number U, typename = std::enable_if_t<!std::is_same_v<U,T> && std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>>
    constexpr Line(U slope, T yIntercept):Line(fishnet::math::DEFAULT_NUMERIC(slope),yIntercept){}

    constexpr Line<T> toLine() const noexcept{
        return Line(p,q);
    }

    constexpr Vec2D<T> direction() const noexcept{
        return q-p;
    }

    constexpr bool isVertical() const noexcept{
        return p.x == q.x;
    }

    constexpr std::optional<fishnet::math::DEFAULT_FLOATING_POINT> yIntercept() const noexcept{
        std::optional<Vec2DReal> intersectionWithY = intersection(Y_AXIS);
        [[likely]] if (intersectionWithY){
            return std::optional(intersectionWithY->y);
        }
        return std::nullopt;
    }

    constexpr fishnet::math::DEFAULT_FLOATING_POINT slope() const noexcept{
        [[unlikely]] if (direction().x == 0) 
            return std::numeric_limits<fishnet::math::DEFAULT_FLOATING_POINT>::max();
        return fishnet::math::DEFAULT_FLOATING_POINT(direction().y) / fishnet::math::DEFAULT_FLOATING_POINT(direction().x);
    }

    constexpr bool contains(IPoint auto const & point) const noexcept{
        using FLOAT_TYPE = fishnet::math::DEFAULT_FLOATING_POINT;
        auto dir = direction();
        if(dir.x == 0) { // line is vertical
            return point.x == p.x; //or q.x
        }
        if(dir.y == 0){ // line is horizontal
            return point.y == p.y;
        }
        FLOAT_TYPE lX = FLOAT_TYPE(point.x - this->p.x) / FLOAT_TYPE(dir.x);
        FLOAT_TYPE lY = FLOAT_TYPE(point.y - this->p.y) / FLOAT_TYPE(dir.y);
        return fabs(lX-lY) < fishnet::math::EPSILON;
    }

    constexpr bool isLeft(IPoint auto const & point) const noexcept {
        return direction().cross(point-p) > 0;
    }

    constexpr bool isRight(IPoint auto const & point) const noexcept {
        return direction().cross(point-p) < 0;
    }

    constexpr bool isParallel(LinearGeometry auto const& other) const noexcept {
        return areParallel(*this,other);
    }

    constexpr bool intersects(LinearGeometry auto const& other) const noexcept{
        return intersect(*this,other);
    }

    template<fishnet::math::Number U>
    constexpr bool operator==(const Line<U> & other) const noexcept {
        if(this->isParallel(other)){
            [[unlikely]] if (isVertical()){ //other must also be vertical since both are parallel
                return this->p.x == other.p.x;
            }
            return fabs(yIntercept().value() - other.yIntercept().value()) < fishnet::math::EPSILON;
        }
        return false;
    }

    constexpr std::optional<Vec2DReal> intersection(LinearGeometry auto const& other) const noexcept {
        return linearIntersection(*this,other);
    }

    constexpr std::string toString() const {
        auto t = yIntercept();
        if (t){
            return "Line y = " + std::to_string(slope())+" * x + "+std::to_string(t.value());
        }
        return "Vertical Line x = "+std::to_string(p.x);
    }
};
constexpr Line<double> xAxis = Line<double>::X_AXIS;
constexpr Line<double> yAxis = Line<double>::Y_AXIS;
static_assert(LinearGeometry<Line<double>>);

// Explicit template instantiation
template class Line<fishnet::math::DEFAULT_NUMERIC>;

//Deduction guides
template<math::Number T>
Line(Vec2D<T>,Vec2D<T>) -> Line<T>;
}

namespace std{
    template<typename T>
    struct hash<fishnet::geometry::Line<T>>{
        constexpr static auto hasher = hash<fishnet::math::DEFAULT_FLOATING_POINT>{};
        size_t operator() (const fishnet::geometry::Line<T> & line ) const {
            if (line.isVertical()) return hasher(line.p.x);
            size_t slopeHash = hasher(line.slope());
            size_t yInterceptHash = hasher(line.yIntercept().value());
            return fishnet::util::CantorPairing(slopeHash,yInterceptHash);
        }
    };
}
