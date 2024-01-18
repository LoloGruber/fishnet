#pragma once

#include "Segment.hpp"
#include "InvalidGeometryException.hpp"

#include "CollectionConcepts.hpp"

namespace fishnet::geometry {

template<fishnet::math::Number T>
constexpr static bool isValidPolygonalRing(util::random_access_range_of<Segment<T>> auto const & segments) noexcept{
    if (segments.size() < 3) 
        return false;
    for(size_t i = 0 ; i < segments.size();++i) {
        auto l = segments[i];
        if(not l.isValid()) continue;
        for (size_t j = i+2; j< segments.size(); ++j){
            auto r = segments[j];
            if(not r.isValid()) continue;
            if (l.intersects(r) and not l.touches(r)){
                return false;
            }
        }
        if(not segments[i].touches(segments[(i+1)%segments.size()])) 
            return false; // adjacent segments have to touch each other
    }
    return true;
}

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
