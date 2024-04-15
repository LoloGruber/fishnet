#pragma once
#include "GeometryType.hpp"
#include "ShapeGeometry.hpp"
#include "LinearGeometry.hpp"
#include "Vec2D.hpp"
#include <fishnet/HashConcepts.hpp>


namespace fishnet::geometry {
#include <fishnet/Printable.hpp>
template<typename G, typename T = typename G::numeric_type>
concept GeometryObject = (std::convertible_to<G,Vec2D<T>> || LinearGeometry<G,T> || ShapeGeometry<G,T>) && Printable<G> && util::Hashable<G> && requires(const G & geometry){
    {G::type} -> std::convertible_to<GeometryType>;
    typename G::numeric_type;
};

template<typename O, typename G = typename O::value_type>
concept OptionalGeometryObject = (std::convertible_to<O,std::optional<G>> && GeometryObject<G>);
}