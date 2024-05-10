#pragma once

#include <fishnet/Segment.hpp>
#include <fishnet/InvalidGeometryException.hpp>

#include <fishnet/CollectionConcepts.hpp>

namespace fishnet::geometry {

/**
 * @brief Decides whether a range of segments is a valid, closed polygonal ring
 * 
 * @tparam T numeric type of the segments
 * @param segments range of segments
 * @return true, segments form a valid polygonal ring
 * @return false, not a polygonal ring
 */
template<fishnet::math::Number T>
constexpr static bool isValidPolygonalRing(util::random_access_range_of<Segment<T>> auto const & segments) noexcept{
    if (segments.size() < 3) 
        return false; // a ring has to have at least three segments
    for(size_t i = 0 ; i < segments.size();++i) {
        auto l = segments[i];
        if(not l.isValid()) 
            continue; // 0-length segments get skipped
        for (size_t j = i+2; j< segments.size(); ++j){
            auto r = segments[j];
            if(not r.isValid()) 
                continue; // 0-length segments get skipped
            if (l.intersects(r) and not l.touches(r)){
                return false; // if segments intersect, and the intersection is not an endpoint of both segments, the ring is not valid (->Self-Intersection)
            }
        }
        if(not segments[i].touches(segments[(i+1)%segments.size()])) 
            return false; // adjacent segments have to touch each other (-> Closed Ring)
    }
    return true;
}

/**
 * @brief Verify a range segments is a valid, closed polygonal ring
 * 
 * @tparam T numeric type of the segments
 * @param segments range of segments
 * @throws InvalidGeometryException when the segments do not form a valid polygonal ring
 */
template<fishnet::math::Number T>
constexpr static void verifyPolygonalRing(util::random_access_range_of<Segment<T>> auto const & segments) {
    if (segments.size() < 3) 
        throw InvalidGeometryException("Ring has to contain at least three Segments");
    for(size_t i = 0 ; i < segments.size();++i) {
        auto l = segments[i];
        if(not l.isValid()) continue;
        for (size_t j = i+2; j< segments.size(); ++j){
            auto r = segments[j];
            if(not r.isValid()) continue;
            if (l.intersects(r) and not l.touches(r))
                throw InvalidGeometryException("Ring Segments intersect each other: \n"+l.toString()+" intersects "+r.toString());
        }
        if(not segments[i].touches(segments[(i+1)%segments.size()])) 
            throw InvalidGeometryException("Adjacent Segments of Ring do not touch at endpoints: \n"+segments[i].toString()+" does not touch "+segments[(i+1)%segments.size()].toString());
    }
}

}
