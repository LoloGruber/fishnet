#pragma once
#include <fishnet/Option.hpp>
#include <fishnet/Vec2D.hpp>
#include "OGRAdapterBase.hpp"
#include "OGRRingAdapter.hpp"
#include "OGRPolygonAdapter.hpp"
#include "OGRMultiPolygonAdapter.hpp"

namespace fishnet::geometry{

class OGRGeometryAdapter;

/**
 * @brief A geometry an OGR layer can be read into and written from
 *
 * The shapes are backed by the OGR geometry of the data source and copy nothing out of it. A point
 * is the exception: it is two doubles, so Vec2DReal already is the cheapest representation there
 * is and wrapping an OGRPoint behind a pointer would only cost an allocation.
 *
 * OGRGeometryAdapter itself is included for layers which do not care what they hold: it keeps every
 * geometry of the source, including the ones fishnet has no type for, and is what a pass through
 * (merge, split, filter by attribute) should use. Its geometries carry no static GeometryType, so
 * they have to be narrowed before any generic algorithm can be run on them.
 */
template<typename G>
concept OGRLayerGeometry =
       std::same_as<G, Vec2DReal>
    || std::same_as<G, OGRRingAdapter>
    || std::same_as<G, OGRPolygonAdapter>
    || std::same_as<G, OGRMultiPolygonAdapter>
    || std::same_as<G, OGRGeometryAdapter>;

/**
 * @brief A geometry which can be written to an OGR data source
 *
 * Wider than OGRLayerGeometry, which is what a layer can be *read* into: reading has to build the
 * geometry from the source, whereas writing starts from one which already exists, and a fishnet
 * value type converts into an OGR geometry just as well as an adapter hands its own over. Segments,
 * rays and lines are left out, as WKB has no representation for them.
 */
template<typename G>
concept OGRWritableGeometry = IPoint<G> || Shape<G> || DynamicGeometry<G>;

namespace __impl{
    // overload set used only in an unevaluated context (@see NarrowTarget below) to compute which
    // concrete adapter narrowTo() should produce for a given G: itself for one of the adapters (or
    // Vec2DReal), otherwise whichever adapter models the same kind of shape as G
    template<OGRLayerGeometry G>
    G narrowTargetOf();

    template<typename G> requires (not OGRLayerGeometry<G>) && IRing<G>
    OGRRingAdapter narrowTargetOf();

    template<typename G> requires (not OGRLayerGeometry<G>) && IPolygon<G>
    OGRPolygonAdapter narrowTargetOf();

    template<typename G> requires (not OGRLayerGeometry<G>) && IMultiPolygon<G>
    OGRMultiPolygonAdapter narrowTargetOf();
}

/**
 * @brief What narrowTo<G>() produces: G itself for one of the OGRLayerGeometry types, otherwise
 * whichever OGR adapter models the same kind of shape as G
 *
 * This is what lets narrowTo() also accept a plain fishnet::geometry::Ring/Polygon/MultiPolygon
 * (or any other IRing/IPolygon/IMultiPolygon), and not just the OGR adapters listed there.
 */
template<typename G>
using NarrowTarget = decltype(__impl::narrowTargetOf<G>());

/**
 * @brief Adapter for an OGRGeometry of statically unknown type, as handed out by a data source
 *
 * Narrows the wrapped geometry to one of the concrete adapters. Every conversion verifies the WKB
 * type first and yields an empty Option if the geometry is of another type (or empty), so that a
 * geometry read from a data source can be dispatched without an unchecked downcast:
 *
 *      OGRGeometryAdapter geometry {std::move(geometryPtr)};
 *      geometry.toPolygon().if_value([](const auto & polygon){ ... });
 *
 * The conversions copy the wrapped geometry, leaving this adapter intact.
 */
class OGRGeometryAdapter: public OGRAdapterBase<OGRGeometry>{
private:
    using Base = OGRAdapterBase<OGRGeometry>;
public:
    using numeric_type = double;

    OGRGeometryAdapter(OGRUniquePtr<OGRGeometry> && geometryPtr):Base(std::move(geometryPtr)) {}

    OGRGeometryAdapter(const OGRRingAdapter & ring):Base(OGRUniquePtr<OGRGeometry>(ring.raw()->getExteriorRing()->clone())) {}

    OGRGeometryAdapter(const OGRPolygonAdapter & polygon):Base(OGRUniquePtr<OGRGeometry>(polygon.raw()->clone())) {}

    OGRGeometryAdapter(const OGRMultiPolygonAdapter & multiPolygon):Base(OGRUniquePtr<OGRGeometry>(multiPolygon.raw()->clone())) {}

    /**
     * @brief Adopt a ring, polygon or multi-polygon which is no longer needed, without cloning it
     * @note the source adapter is left moved-from and must not be used again
     */
    OGRGeometryAdapter(OGRRingAdapter && ring):Base(std::move(ring).releaseGeometry()) {}

    OGRGeometryAdapter(OGRPolygonAdapter && polygon):Base(std::move(polygon).releaseGeometry()) {}

    OGRGeometryAdapter(OGRMultiPolygonAdapter && multiPolygon):Base(std::move(multiPolygon).releaseGeometry()) {}

    /**
     * @brief Build from any ring, polygon or multi-polygon which is not already one of the OGR
     * adapters above, by first converting it through the corresponding adapter
     *
     * The adapter does the actual conversion (walking the shape's points/rings); the temporary it
     * produces is then adopted by the move constructor above rather than cloned.
     */
    OGRGeometryAdapter(const IRing auto & ring):OGRGeometryAdapter(OGRRingAdapter(ring)) {}

    OGRGeometryAdapter(const IPolygon auto & polygon):OGRGeometryAdapter(OGRPolygonAdapter(polygon)) {}

    OGRGeometryAdapter(const IMultiPolygon auto & multiPolygon):OGRGeometryAdapter(OGRMultiPolygonAdapter(multiPolygon)) {}

    /**
     * @brief Geometry type of the wrapped geometry
     * @note OGR reports wkbLineString for an OGRLinearRing, as a linear ring has no WKB type of
     * its own; @see isRing()
     */
    GeometryType geometryType() const noexcept {
        return GeometryTypeWKBAdapter::fromWKB(wkbFlatten(geomPtr->getGeometryType()));
    }

    bool isEmpty() const noexcept {
        return geomPtr->IsEmpty();
    }

    /**
     * @brief Whether the wrapped geometry can be viewed as a ring
     * @note a linear ring is reported as wkbLineString by OGR, so a line string with enough points
     * to enclose an area is accepted here; it is closed on conversion, just like every other ring
     * handed to OGRRingAdapter
     */
    bool isRing() const noexcept {
        auto wkbType = wkbFlatten(geomPtr->getGeometryType());
        return (wkbType == wkbLineString || wkbType == wkbLinearRing)
            && not isEmpty() && geomPtr->toLineString()->getNumPoints() >= 3;
    }

    bool isPolygon() const noexcept {
        return wkbFlatten(geomPtr->getGeometryType()) == wkbPolygon && not isEmpty();
    }

    bool isMultiPolygon() const noexcept {
        return wkbFlatten(geomPtr->getGeometryType()) == wkbMultiPolygon && not isEmpty();
    }

    bool isPoint() const noexcept {
        return wkbFlatten(geomPtr->getGeometryType()) == wkbPoint && not isEmpty();
    }

    /**
     * @brief Read the wrapped point out into a Vec2DReal
     * @return empty Option if the wrapped geometry is not a point
     * @note unlike the shapes there is nothing to adapt here: a point is two doubles, so it is read
     * out by value rather than kept behind a pointer
     */
    Option<Vec2DReal> toPoint() const {
        if(not isPoint())
            return {};
        const auto * point = geomPtr->toPoint();
        return Vec2DReal(point->getX(), point->getY());
    }

    /**
     * @brief Convert to an OGRRingAdapter
     * @return empty Option if the wrapped geometry is not a ring
     */
    Option<OGRRingAdapter> toRing() const {
        if(not isRing())
            return {};
        const auto * lineString = geomPtr->toLineString();
        auto ring = OGRUniquePtr<OGRLinearRing>(static_cast<OGRLinearRing*>(OGRGeometryFactory::createGeometry(wkbLinearRing)));
        for(int i = 0; i < lineString->getNumPoints(); ++i){
            ring->addPoint(lineString->getX(i), lineString->getY(i));
        }
        return OGRRingAdapter(std::move(ring));
    }

    /**
     * @brief Convert to an OGRPolygonAdapter
     * @return empty Option if the wrapped geometry is not a polygon
     */
    Option<OGRPolygonAdapter> toPolygon() const & {
        if(not isPolygon())
            return {};
        return OGRPolygonAdapter(OGRUniquePtr<OGRPolygon>(geomPtr->toPolygon()->clone()));
    }

    /**
     * @brief Hand the wrapped polygon over to an OGRPolygonAdapter, without copying it
     *
     * For a geometry this adapter owns and the caller does not need afterwards, e.g. one just taken
     * from a data source. On success this adapter is left moved-from and must not be used again.
     * @return empty Option if the wrapped geometry is not a polygon, in which case nothing is
     * handed over and this adapter stays intact
     */
    Option<OGRPolygonAdapter> toPolygon() && {
        if(not isPolygon())
            return {};
        auto deleter = geomPtr.get_deleter(); // whoever owns the geometry keeps owning it
        auto * released = geomPtr.release();
        return OGRPolygonAdapter(OGRUniquePtr<OGRPolygon>(released->toPolygon(), deleter));
    }

    /**
     * @brief Convert to an OGRMultiPolygonAdapter
     * @return empty Option if the wrapped geometry is not a multi-polygon
     */
    Option<OGRMultiPolygonAdapter> toMultiPolygon() const & {
        if(not isMultiPolygon())
            return {};
        return OGRMultiPolygonAdapter(OGRUniquePtr<OGRMultiPolygon>(geomPtr->toMultiPolygon()->clone()));
    }

    /**
     * @brief Hand the wrapped multi-polygon over to an OGRMultiPolygonAdapter, without copying it
     * @see toPolygon() && for the ownership semantics
     */
    Option<OGRMultiPolygonAdapter> toMultiPolygon() && {
        if(not isMultiPolygon())
            return {};
        auto deleter = geomPtr.get_deleter();
        auto * released = geomPtr.release();
        return OGRMultiPolygonAdapter(OGRUniquePtr<OGRMultiPolygon>(released->toMultiPolygon(), deleter));
    }


    /**
     * @brief Narrow to G, leaving this adapter intact
     * @note where the result has to own a geometry of its own this copies the wrapped one,
     * @see narrowTo() && for the version which hands it over instead
     */
    template<typename G> requires OGRLayerGeometry<G> || Shape<G>
    Option<NarrowTarget<G>> narrowTo() const & {
        using Target = NarrowTarget<G>;
        // a point is read out by value and a ring is rebuilt from the points either way, so neither
        // gains anything from copying the whole geometry first
        if constexpr (std::same_as<Target, Vec2DReal>)
            return toPoint();
        else if constexpr (std::same_as<Target, OGRRingAdapter>)
            return toRing();
        else
            return OGRGeometryAdapter(*this).narrowTo<G>();
    }


    /**
     * @brief Narrow to G, coercing between polygon and multi-polygon where the shape is the same
     *
     * G is not limited to the OGR adapters: any other IRing/IPolygon/IMultiPolygon (a plain
     * fishnet::geometry::Ring/Polygon/MultiPolygon, for instance) narrows to whichever adapter
     * models the same kind of shape, @see NarrowTarget.
     *
     * A data source does not always hold the geometry type its layer declares: a polygon layer may
     * carry a multi-polygon of a single part, and a multi-polygon layer a lone polygon. Both denote
     * the same shape, so they are converted rather than dropped. Anything else yields an empty
     * Option, and the geometry is left where it was.
     * @note on success this adapter is left moved-from and must not be used again
     */
    template<typename G> requires OGRLayerGeometry<G> || Shape<G>
    Option<NarrowTarget<G>> narrowTo() && {
        using Target = NarrowTarget<G>;
        if constexpr (std::same_as<Target, OGRGeometryAdapter>){
            return std::move(*this); // a layer which takes anything has nothing to narrow
        } else if constexpr (std::same_as<Target, Vec2DReal>){
            return toPoint();
        } else if constexpr (std::same_as<Target, OGRRingAdapter>){
            return toRing();
        } else if constexpr (std::same_as<Target, OGRPolygonAdapter>){
            if(auto polygon = std::move(*this).toPolygon())
                return polygon;
            auto multiPolygon = std::move(*this).toMultiPolygon();
            if(not multiPolygon || multiPolygon.value().size() != 1)
                return {};
            auto polygons = multiPolygon.value().getPolygons();
            return *std::ranges::begin(polygons);
        } else if constexpr (std::same_as<Target, OGRMultiPolygonAdapter>){
            if(auto multiPolygon = std::move(*this).toMultiPolygon())
                return multiPolygon;
            auto polygon = std::move(*this).toPolygon();
            if(not polygon)
                return {};
            std::vector<OGRPolygonAdapter> parts;
            parts.push_back(std::move(polygon.value()));
            return OGRMultiPolygonAdapter(parts, true);
        } else {
            static_assert(false, "narrowTo has no case for this geometry: a type was added to "
                "OGRLayerGeometry without saying how an OGR geometry is narrowed to it");
        }
    }
};

static_assert(OGRWritableGeometry<OGRPolygonAdapter>);
static_assert(OGRWritableGeometry<OGRGeometryAdapter>);
static_assert(OGRLayerGeometry<Vec2DReal>);
static_assert(OGRLayerGeometry<OGRRingAdapter>);
static_assert(OGRLayerGeometry<OGRPolygonAdapter>);
static_assert(OGRLayerGeometry<OGRMultiPolygonAdapter>);
static_assert(OGRLayerGeometry<OGRGeometryAdapter>);
}
