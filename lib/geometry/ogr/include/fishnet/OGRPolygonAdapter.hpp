#pragma once
#include <numeric>
#include <vector>
#include <fishnet/IGeometry.hpp>
#include <fishnet/Rectangle.hpp>
#include <fishnet/PolygonDistance.hpp>
#include "OGRAdapterBase.hpp"
#include "OGRRingAdapter.hpp"

namespace fishnet::geometry{

/**
 * @brief IPolygon implementation backed by an OGRPolygon
 *
 * The exterior ring forms the boundary, the interior rings are the holes. Unlike a bare
 * OGRLinearRing, an OGRPolygon is a first class GEOS geometry, so the spatial predicates can be
 * delegated to OGR directly. Where OGR/OGC and fishnet disagree, fishnet's convention wins, so that
 * an OGR backed polygon is substitutable for a fishnet::geometry::Polygon:
 * - contains() includes the boundary, whereas OGR's Contains() is interior only
 * - centroid() is the area weighted centroid over the mean-of-points centroids of its rings
 * - distance() returns -1 for a contained polygon and 0 for a touching one
 */
class OGRPolygonAdapter: public OGRAdapterBase<OGRPolygon>{
private:
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
     * @brief Copy an arbitrary IPolygon into an OGRPolygon, ready to be used with GEOS
     */
    static OGRPolygon toOGRPolygon(const IPolygon auto & polygon) {
        OGRPolygon ogrPolygon;
        auto boundary = toLinearRing(polygon.getBoundary());
        ogrPolygon.addRing(&boundary);
        for(const auto & hole : polygon.getHoles()){
            auto ogrHole = toLinearRing(hole);
            ogrPolygon.addRing(&ogrHole);
        }
        return ogrPolygon;
    }

    static OGRPolygon toOGRPolygon(const IRing auto & ring) {
        OGRPolygon ogrPolygon;
        auto boundary = toLinearRing(ring);
        ogrPolygon.addRing(&boundary);
        return ogrPolygon;
    }

    /**
     * @brief Boundary inclusive containment, @see OGRRingAdapter
     */
    static bool covers(const OGRGeometry & covering, const OGRGeometry & covered) {
        auto difference = OGRUniquePtr<OGRGeometry>(covered.Difference(&covering));
        return difference && difference->IsEmpty();
    }

    OGRLinearRing * exteriorRing() const {
        return geomPtr->getExteriorRing();
    }

public:
    using numeric_type = double;
    static constexpr GeometryType type = GeometryType::POLYGON;

    OGRPolygonAdapter(OGRUniquePtr<OGRPolygon> && polygonPtr):Base(std::move(polygonPtr)) {
        geomPtr->closeRings();
    }

    OGRPolygonAdapter(IPolygon auto const & polygon):Base(emptyPolygon()){
        auto boundary = toLinearRing(polygon.getBoundary());
        geomPtr->addRing(&boundary);
        for(const auto & hole : polygon.getHoles()){
            auto ogrHole = toLinearRing(hole);
            geomPtr->addRing(&ogrHole);
        }
    }

    /**
     * @brief Construct a simple polygon (no holes) from a ring
     */
    template<IRing R> requires (not IPolygon<R>)
    OGRPolygonAdapter(R const & boundary):Base(emptyPolygon()){
        auto ogrBoundary = toLinearRing(boundary);
        geomPtr->addRing(&ogrBoundary);
    }

    /**
     * @brief Construct a polygon from a boundary ring and its holes
     */
    OGRPolygonAdapter(const IRing auto & boundary, const RingRange auto & holes):Base(emptyPolygon()){
        auto ogrBoundary = toLinearRing(boundary);
        geomPtr->addRing(&ogrBoundary);
        for(const auto & hole : holes){
            auto ogrHole = toLinearRing(hole);
            geomPtr->addRing(&ogrHole);
        }
    }

    OGRRingAdapter getBoundary() const {
        return OGRRingAdapter(OGRUniquePtr<OGRLinearRing>(exteriorRing()->clone()));
    }

    std::vector<OGRRingAdapter> getHoles() const {
        std::vector<OGRRingAdapter> holes;
        holes.reserve(static_cast<size_t>(geomPtr->getNumInteriorRings()));
        for(int i = 0; i < geomPtr->getNumInteriorRings(); ++i){
            holes.emplace_back(OGRUniquePtr<OGRLinearRing>(geomPtr->getInteriorRing(i)->clone()));
        }
        return holes;
    }

    bool isSimple() const {
        return geomPtr->getNumInteriorRings() == 0;
    }

    /**
     * @brief Area enclosed by the boundary, reduced by the area of the holes
     */
    double area() const {
        return geomPtr->get_Area();
    }

    /**
     * @brief Area weighted centroid of the polygon, obtained by decomposition into its rings
     * @note fishnet defines the centroid of a ring as the mean of its points, which is what the
     * decomposition below is built on; it is not the geometric centroid OGR/GEOS would compute.
     */
    Vec2DReal centroid() const {
        auto boundary = getBoundary();
        auto totalAreaIncludingHoles = boundary.area();
        auto accumulatedCentroid = boundary.centroid() * totalAreaIncludingHoles;
        auto accumulatedArea = totalAreaIncludingHoles;
        for(const auto & hole : getHoles()){
            accumulatedCentroid = accumulatedCentroid + hole.centroid() * -hole.area();
            accumulatedArea -= hole.area();
        }
        return accumulatedCentroid / accumulatedArea;
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
     * @note the boundary of a polygon comprises its exterior ring and the rings of its holes
     */
    bool isOnBoundary(const IPoint auto & point) const {
        OGRPoint ogrPoint(point.getX(), point.getY());
        return geomPtr->Touches(&ogrPoint);
    }

    bool isOutside(const IPoint auto & point) const {
        OGRPoint ogrPoint(point.getX(), point.getY());
        return geomPtr->Disjoint(&ogrPoint);
    }

    bool containsInHole(const IPoint auto & point) const {
        auto holes = getHoles();
        return std::ranges::any_of(holes, [&point](const auto & hole){return hole.contains(point);});
    }

    bool containsInHole(const IPolygon auto & other) const {
        auto holes = getHoles();
        return std::ranges::any_of(holes, [&other](const auto & hole){return hole.contains(other.getBoundary());});
    }

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
        return covers(*geomPtr, toOGRPolygon(ring));
    }

    bool contains(const IPolygon auto & other) const {
        return covers(*geomPtr, toOGRPolygon(other));
    }

    bool contains(const OGRPolygonAdapter & other) const {
        return covers(*geomPtr, *other.geomPtr);
    }

    bool intersects(const LinearGeometry auto & linearFeature) const {
        if(getBoundary().intersects(linearFeature))
            return true;
        auto holes = getHoles();
        return std::ranges::any_of(holes, [&linearFeature](const auto & hole){return hole.intersects(linearFeature);});
    }

    std::unordered_set<Vec2DReal> intersections(const LinearGeometry auto & linearFeature) const {
        std::unordered_set<Vec2DReal> intersectionSet;
        for(const auto & point : getBoundary().intersections(linearFeature)){
            intersectionSet.insert(point);
        }
        for(const auto & hole : getHoles()){
            for(const auto & point : hole.intersections(linearFeature)){
                intersectionSet.insert(point);
            }
        }
        return intersectionSet;
    }

    bool crosses(const IPolygon auto & other) const {
        if(containsInHole(other))
            return false;
        if(getBoundary().crosses(other.getBoundary()))
            return true;
        auto holes = getHoles();
        return std::ranges::any_of(holes, [&other](const auto & hole){return hole.crosses(other.getBoundary());});
    }

    bool touches(const IPolygon auto & other) const {
        if(this->crosses(other))
            return false;
        if(getBoundary().touches(other.getBoundary()))
            return true;
        auto holes = getHoles();
        return std::ranges::any_of(holes, [&other](const auto & hole){
            // a hole fully contains the other polygon and touches it at least at one point
            return hole.contains(other.getBoundary()) && std::ranges::any_of(other.getBoundary().getPoints(),[&hole](const auto & p){
                return hole.isOnBoundary(p);
            });
        });
    }

    double distance(const IPolygon auto & other) const {
        if(this->contains(other))
            return -1;
        if(this->touches(other))
            return 0;
        for(const auto & hole : getHoles()){
            if(hole.contains(other.getBoundary())){
                return shapeDistance(hole, other.getBoundary());
            }
        }
        return getBoundary().distance(other.getBoundary());
    }

    /**
     * @brief Equality: equal boundaries and equal holes, regardless of their order or of the
     * starting vertex of any ring (matching fishnet::geometry::Polygon)
     * @note OGR's own Equals() compares coordinate sequences instead and would report two
     * rotations of one polygon as different.
     */
    bool operator==(const OGRPolygonAdapter & other) const noexcept {
        if(geomPtr == other.geomPtr)
            return true;
        auto symmetricDifference = OGRUniquePtr<OGRGeometry>(geomPtr->SymDifference(other.geomPtr.get()));
        return symmetricDifference && symmetricDifference->IsEmpty();
    }

    /**
     * @brief Hash compatible with fishnet::geometry::Polygon
     */
    size_t hash() const noexcept {
        auto holeHashes = getHoles() | std::views::transform([](const auto & hole){return hole.hash();});
        size_t holesHash = std::accumulate(std::ranges::begin(holeHashes), std::ranges::end(holeHashes), size_t(0));
        return getBoundary().hash() ^ holesHash;
    }
};
static_assert(IPolygon<OGRPolygonAdapter>);
static_assert(Shape<OGRPolygonAdapter>);
}
