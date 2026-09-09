#pragma once
#include <fishnet/Vec2D.hpp>
#include <fishnet/Option.hpp>
#include <fishnet/IGeometry.hpp>

namespace fishnet::geometry{

/**
 * @brief Computation of line intersection
 * @param l line object
 * @param r line object
 * @return fishnet::Option<Vec2DReal>: empty optional if lines are parallel, otherwise the intersection of the lines l and r
 * 
 */
constexpr static fishnet::Option<Vec2DReal> inline linearIntersection(ILine auto const& l, ILine auto const & r) noexcept {
    if (l.isParallel(r)) 
        return {};
    auto dThis = Vec2DReal(l.direction());
    const auto & p = l.p();
    const auto & q = l.q();
    const auto & s = r.p();
    const auto & t = r.q();
    using namespace fishnet::math;
    DEFAULT_FLOATING_POINT denominator = (p.x - q.x) * (s.y - t.y) - (p.y - q.y) *(s.x - t.x);
    DEFAULT_FLOATING_POINT lambda = ((p.x - s.x) * (s.y - t.y) - (p.y - s.y) * (s.x - t.x)) /denominator;
    Vec2DReal intersectionOfLines =  p + (dThis * lambda);
    return intersectionOfLines;
}

/**
 * @brief Generic wrapper for intersections between linear features
 * 
 * @param lhs linear geometry
 * @param rhs linear geometry
 * @return fishnet::Option<Vec2DReal>
 */
constexpr static fishnet::Option<Vec2DReal> inline linearIntersection(LinearGeometry auto const& lhs, LinearGeometry auto const& rhs) requires(!ILine<decltype(lhs)> || !ILine<decltype(rhs)>) {
    return linearIntersection(lhs.toLine(),rhs.toLine()).filter([&lhs,&rhs](const auto & intersection){return lhs.contains(intersection) && rhs.contains(intersection);});
}

constexpr static bool inline areParallel(LinearGeometry auto const & lhs, LinearGeometry auto const & rhs) noexcept {
    return lhs.direction().isParallel(rhs.direction());
}

constexpr static bool intersect(LinearGeometry auto const & lhs,LinearGeometry auto const& rhs) noexcept {
    return linearIntersection(lhs,rhs).has_value();
}
}