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

template<typename S>
concept Shape = true;

static_assert(Shape<__impl::RingStub<double>>);
static_assert(Shape<__impl::PolygonStub<double>>);
static_assert(Shape<__impl::MultiPolygonStub<double>>);

template<typename G>
concept Geometry = IPoint<G> || LinearGeometry<G> || Shape<G>;

static_assert(Geometry<__impl::PointStub<double>>);
static_assert(Geometry<__impl::LineStub<double>>);
static_assert(Geometry<__impl::SegmentStub<double>>);
static_assert(Geometry<__impl::RayStub<double>>);
static_assert(Geometry<__impl::RingStub<double>>);
static_assert(Geometry<__impl::PolygonStub<double>>);
static_assert(Geometry<__impl::MultiPolygonStub<double>>);
} // namespace fishnet::geometry