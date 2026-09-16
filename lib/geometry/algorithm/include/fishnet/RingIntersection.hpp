#pragma once
#include <ranges>
#include <fishnet/IGeometry.hpp>

namespace fishnet::geometry {

/**
 * @brief test whether a linear feature intersects a ring
 * Check every intersection of the linear feature with the segments of the ring:
 * If the endpoints of the intersected segment are on opposite sides of the linear feature -> TRUE
 * If intersection point is vertex of the ring:
 *      s1.p()-----------s1.q() == intersection point == s2.p()----------s2.q()
 *      -> Test if s1.p() and s2.() are on the same side of the linear feature, if not it must be a intersection -> TRUE
 * Additional checks depending on type linear feature:
 * - L == ISegment:
 *      skip if intersection is endpoint of both the segment of the boundary and the linear feature
 *      otherwise test if any of the endpoints of the linear feature are outside of the ring -> TRUE
 * - L == IRing:
 *      skip if intersection is the origin of the ray
 *
 * Shared by all IRing implementations, such that a ring backed by e.g. an OGRLinearRing reports
 * the same intersections as fishnet::geometry::Ring.
 *
 * @param ring ring object, its getSegments() must be a random access range
 * @param linearFeature
 */
template<typename R, LinearGeometry L>
constexpr static bool ringIntersects(const R & ring, const L & linearFeature) noexcept {
    auto segments = ring.getSegments();
    using difference_type = std::ranges::range_difference_t<decltype(segments)>;
    auto size = std::ranges::ssize(segments);
    auto segmentAt = [&segments](difference_type index){return *(std::ranges::begin(segments) + index);};
    // Helper lambda to check whether two points are on the same side of the linearFeature (or on the line)
    auto onSameSide = [&linearFeature](const auto & lhs, const auto & rhs) {
        auto line = linearFeature.toLine();
        if(line.contains(rhs) || line.contains(lhs)){
            return true;
        }
        return line.isLeft(lhs) == line.isLeft(rhs);
    };
    for(difference_type i = 0; i < size; ++i){
        auto current = segmentAt(i);
        auto inter = current.intersection(linearFeature);
        if constexpr(ISegment<L>){
            if(inter && (linearFeature.isEndpoint(inter.value()) && current.isEndpoint(inter.value())))
                 continue;
            if(inter && (linearFeature.isEndpoint(inter.value())) && (ring.isOutside(linearFeature.p()) || ring.isOutside(linearFeature.q())))
                return true;
        }
        if constexpr(IRay<L>){
            if(inter && inter.value() == linearFeature.origin())
                 continue;
        }
        if(inter && current.isEndpoint(inter.value())) { // intersection is vertex of ring
            if(current.p() == inter.value()){
                // i+size-1 rather than i-1: the latter underflows for i == 0 and then only wraps
                // to size-1 when the number of segments happens to be a power of two
                if(not onSameSide(current.q(),segmentAt((i+size-1)%size).p()))
                    return true;
            }else{ // inter.value() == current.q()
                if(not onSameSide(current.p(),segmentAt((i+1)%size).q()))
                    return true;
            }
        }else if(inter){
            if(not onSameSide(current.p(),current.q())) return true;
        }
    }
    return false;
}
} // namespace fishnet::geometry
