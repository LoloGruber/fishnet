#pragma once
#include <fishnet/Vec2D.hpp>
#include <optional>
#include <numeric>
#include <fishnet/LinearGeometry.hpp>
#include <fishnet/LinearIntersection.hpp>

namespace fishnet::geometry{
template<fishnet::math::Number T = fishnet::math::DEFAULT_NUMERIC>
class Line{
public:
    const Vec2D<T> p;
    const Vec2D<T> q;

    constexpr static Line<T> X_AXIS = Line<T>(T(0),T(0));
    constexpr static Line<T> Y_AXIS = Line<T>(Vec2D<T>(0,0),Vec2D<T>(0,1));

    constexpr static Line<T> verticalLine(T x) noexcept {
        return Line<T>(Vec2D<T>(x,0),Vec2D<T>(x,1));
    }

    constexpr static Line<T> horizontalLine(T y) noexcept {
        return Line<T>(0,y);
    }

    using numeric_type =  T;
    constexpr static GeometryType type = GeometryType::LINE;

    constexpr Line(Vec2D<T> p , Vec2D<T> q):p(p),q(q){
        if (p == q) 
            throw std::invalid_argument("Coinciding Points cannot define a Line");
    }

    template<fishnet::math::Number U, typename = std::enable_if_t<!std::is_same_v<U,T> && std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>>
    constexpr Line(Vec2D<T> p, Vec2D<U> q):Line(p,static_cast<Vec2D<T>>(q)){}

    template<fishnet::math::Number U, typename = std::enable_if_t<!std::is_same_v<U,T> && std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>>
    constexpr Line(Vec2D<U> p , Vec2D<T> q):Line(static_cast<Vec2D<T>>(p),q){}

    constexpr Line(T slope, T yIntercept):p(Vec2D<T>(0,yIntercept)),q(Vec2D<T>(1,slope+yIntercept)){}

    template<fishnet::math::Number U, typename = std::enable_if_t<!std::is_same_v<U,T> && std::is_same_v<T,fishnet::math::DEFAULT_NUMERIC>>>
    constexpr Line(T slope, U yIntercept):Line(slope,fishnet::math::DEFAULT_NUMERIC(yIntercept)){}

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
        using namespace fishnet::math;
        [[unlikely]] if (direction().x == 0) return std::numeric_limits<fishnet::math::DEFAULT_FLOATING_POINT>::max();
        return DEFAULT_FLOATING_POINT(direction().y) / DEFAULT_FLOATING_POINT(direction().x);
    }

    constexpr bool contains(IPoint auto const & point) const noexcept{
        using namespace fishnet::math;
        auto dir = direction();
        if(dir.x == 0) {
            return point.x == p.x; //or q.x
        }
        if(dir.y == 0){
            return point.y == p.y;
        }
        DEFAULT_FLOATING_POINT lX = DEFAULT_FLOATING_POINT(point.x - p.x) / DEFAULT_FLOATING_POINT(dir.x);
        DEFAULT_FLOATING_POINT lY = DEFAULT_FLOATING_POINT(point.y - p.y) / DEFAULT_FLOATING_POINT(dir.y);
        return fabs(lX-lY) < EPSILON;
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
            [[unlikely]] if (isVertical()){ //other must also be vertical since both are vertical
                return this->p.x == other.p.x;
            }
            return fabs(yIntercept().value() - other.yIntercept().value()) < fishnet::math::EPSILON;
        }
        return false;
    }

    constexpr std::optional<Vec2DReal> intersection(LinearGeometry auto const& other) const noexcept {
        // if(isParallel(other)) return std::nullopt;
        // auto dThis = direction();
        // double denominator = (p.x - q.x) * (other.p.y - other.q.y) - (p.y - q.y) *(other.p.x - other.q.x);
        // double lambda = ((p.x - other.p.x) * (other.p.y - other.q.y) - (p.y - other.p.y) * (other.p.x - other.q.x)) /denominator;
        // auto intersectionOfLines =  p + (dThis * lambda);
        // return std::optional(intersectionOfLines);
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
            return util::CantorPairing(slopeHash,yInterceptHash);
        }
    };
}
