#pragma once
#include "IPoint.hpp"
#include "ILine.hpp"
#include "ISegment.hpp"
#include "IRay.hpp"
#include "IRing.hpp"
#include "IPolygon.hpp"
#include "IMultiPolygon.hpp"

namespace fishnet::geometry {

template<typename L>
concept LinearGeometry = ILine<L> || ISegment<L> || IRay<L>;

static_assert(LinearGeometry<__impl::LineStub<double>>);
static_assert(LinearGeometry<__impl::SegmentStub<double>>);
static_assert(LinearGeometry<__impl::RayStub<double>>);

/**
 * @brief A geometry enclosing an area: a ring, a polygon or a multi-polygon
 */
template<typename S>
concept Shape = IRing<S> || IPolygon<S> || IMultiPolygon<S>;

static_assert(Shape<__impl::RingStub<double>>);
static_assert(Shape<__impl::PolygonStub<double>>);
static_assert(Shape<__impl::MultiPolygonStub<double>>);
static_assert(not Shape<double>);

template<typename G>
concept Geometry = IPoint<G> || LinearGeometry<G> || Shape<G>;

/**
 * @brief A geometry which carries its type at runtime instead of statically
 *
 * The counterpart to Geometry: such a geometry models no single shape - it reports which one it
 * holds instead - so it is not usable with the generic algorithms and has to be narrowed to a
 * Geometry first. This is what lets a layer hold whatever its data source happens to contain.
 */
template<typename G>
concept DynamicGeometry = fishnet::util::Object<G> && requires(const G & geometry){
    { geometry.geometryType() } -> std::convertible_to<GeometryType>;
};

/**
 * @brief A geometry of a statically known type, or one which only knows it at runtime
 *
 * What a feature or a layer can store. Deliberately wider than Geometry and narrower than
 * util::Object: a layer does need to hold a type erased geometry, but never a plain string.
 */
template<typename G>
concept AnyGeometry = Geometry<G> || DynamicGeometry<G>;

static_assert(Geometry<__impl::PointStub<double>>);
static_assert(Geometry<__impl::LineStub<double>>);
static_assert(Geometry<__impl::SegmentStub<double>>);
static_assert(Geometry<__impl::RayStub<double>>);
static_assert(Geometry<__impl::RingStub<double>>);
static_assert(Geometry<__impl::PolygonStub<double>>);
static_assert(Geometry<__impl::MultiPolygonStub<double>>);
} // namespace fishnet::geometry