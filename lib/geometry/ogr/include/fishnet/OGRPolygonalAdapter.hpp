#pragma once
#include <fishnet/IGeometry.hpp>
#include <fishnet/Segment.hpp>
#include <fishnet/Rectangle.hpp>
#include "OGRAdapterBase.hpp"

namespace fishnet::geometry{

/**
 * @brief Common base of the OGR backed adapters which enclose an area
 *
 * A ring is a polygon with nothing but an exterior ring, so both OGRRingAdapter and
 * OGRPolygonAdapter are backed by an OGRPolygon and share everything that only depends on the
 * enclosed area. Storing an OGRPolygon rather than an OGRLinearRing is not merely convenient:
 * OGR treats a standalone OGRLinearRing as a second class geometry. It has no WKB representation
 * of its own (WkbSize()/exportToWkb() are private for it) and it cannot be handed to the GEOS
 * backed predicates, which silently answer false / -1 for it instead of reporting an error.
 *
 * The wrapped polygon is kept in the canonical form of GEOS (@see canonicalize()), which makes the
 * coordinate wise OGRAdapterBase::operator== agree with geometric equality for everything but a
 * different vertex density, and thereby keeps it consistent with OGRAdapterBase::hash().
 */
class OGRPolygonalAdapter: public OGRAdapterBase<OGRPolygon>{
protected:
    using Base = OGRAdapterBase<OGRPolygon>;

    static OGRUniquePtr<OGRPolygon> emptyPolygon() {
        return OGRUniquePtr<OGRPolygon>(static_cast<OGRPolygon*>(OGRGeometryFactory::createGeometry(wkbPolygon)));
    }

    static OGRLinearRing toLinearRing(const IRing auto & ring) {
        OGRLinearRing ogrRing;
        for(const auto & point : ring.getPoints()){
            ogrRing.addPoint(point.getX(), point.getY());
        }
        ogrRing.closeRings();
        return ogrRing;
    }

    /**
     * @brief Copy an arbitrary IRing into an OGRPolygon, ready to be used with GEOS
     */
    static OGRUniquePtr<OGRPolygon> toOGRPolygon(const IRing auto & ring) {
        auto polygon = emptyPolygon();
        auto boundary = toLinearRing(ring);
        polygon->addRing(&boundary);
        return polygon;
    }

    /**
     * @brief Copy an arbitrary IPolygon into an OGRPolygon, ready to be used with GEOS
     */
    static OGRUniquePtr<OGRPolygon> toOGRPolygon(const IPolygon auto & polygon) {
        auto ogrPolygon = emptyPolygon();
        auto boundary = toLinearRing(polygon.getBoundary());
        ogrPolygon->addRing(&boundary);
        for(const auto & hole : polygon.getHoles()){
            auto ogrHole = toLinearRing(hole);
            ogrPolygon->addRing(&ogrHole);
        }
        return ogrPolygon;
    }

    /**
     * @brief Boundary inclusive containment: is every point of covered also a point of covering?
     *
     * OGR only offers the strict (interior only) Contains predicate, whereas fishnet counts the
     * boundary as contained. Contains() || Touches() does not express that either, since Touches()
     * also holds when covered merely touches covering from the outside (e.g. two squares sharing a
     * single corner). "covered minus covering is empty" is exactly boundary inclusive containment.
     */
    static bool covers(const OGRGeometry & covering, const OGRGeometry & covered) {
        auto difference = OGRUniquePtr<OGRGeometry>(covered.Difference(&covering));
        return difference && difference->IsEmpty();
    }

    /**
     * @brief Bring the wrapped polygon into the canonical form of GEOS
     *
     * Canonicalising on construction is what lets equality stay a cheap coordinate comparison
     * while still ignoring the starting vertex and the winding order of a ring: two rings which
     * differ only in those are byte identical once normalised. It also keeps equality consistent
     * with the inherited WKB hash, which no amount of comparing geometrically ever could.
     * @note the vertex order of the wrapped geometry is rewritten, its area is not touched
     */
    void canonicalize() noexcept {
        auto normalized = OGRUniquePtr<OGRGeometry>(geomPtr->Normalize());
        if(normalized && wkbFlatten(normalized->getGeometryType()) == wkbPolygon){
            geomPtr.reset(normalized.release()->toPolygon());
        }
    }

    const OGRLinearRing * exteriorRing() const noexcept {
        return geomPtr->getExteriorRing();
    }

    /**
     * @brief Tag for a polygon which is known to be in canonical form already
     * @note deliberately not public: adopting a polygon which is not actually canonical breaks the
     * invariant operator== and hash() rely on
     */
    struct Canonical{};

    /**
     * @brief Adopt a polygon which is already canonical, skipping the GEOS call
     */
    OGRPolygonalAdapter(OGRUniquePtr<OGRPolygon> && polygonPtr, Canonical):Base(std::move(polygonPtr)) {}

    /**
     * @brief Adopt a polygon and bring it into canonical form
     * @note only OGRRingAdapter and OGRPolygonAdapter construct one of these; the base itself is
     * not part of the interface offered to users
     */
    OGRPolygonalAdapter(OGRUniquePtr<OGRPolygon> && polygonPtr):Base(std::move(polygonPtr)) {
        geomPtr->closeRings();
        canonicalize();
    }

public:

    /**
     * @brief Area enclosed by the boundary, reduced by the area of the holes
     */
    double area() const {
        return geomPtr->get_Area();
    }

    Rectangle<double> aaBB() const {
        OGREnvelope boundingBox;
        geomPtr->getEnvelope(&boundingBox);
        return Rectangle<double>(boundingBox.MinX, boundingBox.MaxY, boundingBox.MaxX, boundingBox.MinY);
    }

    bool isInside(const IPoint auto & point) const {
        OGRPoint ogrPoint(point.getX(), point.getY());
        return geomPtr->Contains(&ogrPoint);
    }

    /**
     * @note the boundary comprises the exterior ring as well as the rings of the holes
     */
    bool isOnBoundary(const IPoint auto & point) const {
        OGRPoint ogrPoint(point.getX(), point.getY());
        return geomPtr->Touches(&ogrPoint);
    }

    bool isOutside(const IPoint auto & point) const {
        OGRPoint ogrPoint(point.getX(), point.getY());
        return geomPtr->Disjoint(&ogrPoint);
    }

    /**
     * @note contains is boundary inclusive, unlike OGR's own Contains predicate
     */
    bool contains(const IPoint auto & point) const {
        OGRPoint ogrPoint(point.getX(), point.getY());
        return not geomPtr->Disjoint(&ogrPoint);
    }

    bool contains(const ISegment auto & segment) const {
        OGRLineString ogrLine;
        ogrLine.addPoint(segment.p().getX(), segment.p().getY());
        ogrLine.addPoint(segment.q().getX(), segment.q().getY());
        return covers(*geomPtr, ogrLine);
    }

    bool contains(const IRing auto & ring) const {
        return geomPtr->Contains(toOGRPolygon(ring).get());
    }

    bool contains(const IPolygon auto & polygon) const {
        return geomPtr->Contains(toOGRPolygon(polygon).get());
    }

    bool contains(const OGRPolygonalAdapter & other) const {
        return geomPtr->Contains(other.geomPtr.get());
    }
};
}
