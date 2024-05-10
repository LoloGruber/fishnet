#pragma once
#include <fishnet/Vec2D.hpp>
#include <optional>
#include <fishnet/LinearGeometry.hpp>

namespace fishnet::geometry{

/**
 * @brief Computation of line intersection
 * @param l line object
 * @param r line object
 * @return std::optional<Vec2DReal>: empty optional if lines are parallel, otherwise the intersection of the lines l and r
 * 
 */
constexpr static std::optional<Vec2DReal> inline linearIntersection(ILine auto  const& l, ILine auto  const & r) noexcept {
    if (l.isParallel(r)) 
        return std::nullopt;
    auto dThis = Vec2DReal(l.direction());
    const auto & p = l.p;
    const auto & q = l.q;
    using namespace fishnet::math;
    DEFAULT_FLOATING_POINT denominator = (p.x - q.x) * (r.p.y - r.q.y) - (p.y - q.y) *(r.p.x - r.q.x);
    DEFAULT_FLOATING_POINT lambda = ((p.x - r.p.x) * (r.p.y - r.q.y) - (p.y - r.p.y) * (r.p.x - r.q.x)) /denominator;
    Vec2DReal intersectionOfLines =  p + (dThis * lambda);
    return std::optional(intersectionOfLines);
}

/**
 * @brief Generic wrapper for intersections between linear features
 * 
 * @param lhs linear geometry
 * @param rhs linear geometry
 * @return std::optional<Vec2DReal>
 */
constexpr static std::optional<Vec2DReal> inline linearIntersection(LinearGeometry auto const& lhs, LinearGeometry auto const& rhs) noexcept {
    auto intersection =  linearIntersection(lhs.toLine(),rhs.toLine());
    if (intersection and lhs.contains(*intersection) and rhs.contains(*intersection)) return intersection;
    return std::nullopt;
}

constexpr static bool inline areParallel(LinearGeometry auto const & lhs, LinearGeometry auto const & rhs) noexcept {
    return lhs.direction().isParallel(rhs.direction());
}

constexpr static bool intersect(LinearGeometry auto const & lhs,LinearGeometry auto const& rhs) noexcept {
    return linearIntersection(lhs,rhs).has_value();
}
}