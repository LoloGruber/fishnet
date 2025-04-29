#pragma once
#include "GeometryType.hpp"
#include "ShapeGeometry.hpp"
#include "LinearGeometry.hpp"
#include "Vec2D.hpp"
#include <fishnet/HashConcepts.hpp>
#include <fishnet/Printable.hpp>

namespace fishnet::geometry {

/**
 * @brief Abstract Concept for any Geometry
 * A geometry is either a point, linear feature or shape. Additionally a geometry is required to printable to any ostream and hashable using std::hash.
 * Moreover, every geometry specifies its type and the underlying numeric type (e.g. double, int,...)
 * @tparam G 
 * @tparam G::numeric_type 
 */
template<typename G, typename T = typename G::numeric_type>
concept GeometryObject = (IPoint<G> || LinearGeometry<G,T> || Shape<G,T>) && util::Printable<G> && util::Hashable<G> && requires(const G & geometry){
    {G::type} -> std::convertible_to<GeometryType>;
    typename G::numeric_type;
};
}