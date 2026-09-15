#pragma once
#include <fishnet/IGeometry.hpp>
#include <fishnet/Segment.hpp>
#include <fishnet/Rectangle.hpp>
#include <fishnet/RingIntersection.hpp>
#include <fishnet/PolygonDistance.hpp>
#include "OGRAdapterBase.hpp"

namespace fishnet::geometry{

/**
 * @brief IRing implementation backed by an OGRLinearRing
 *
 * The wrapped ring is always kept closed (first point == last point), as OGR/OGC require it.
 * The IRing facing API however exposes the open representation used throughout fishnet:
 * a ring over the points u,v,x,y,z reports exactly those points and the five segments
 * u-v, v-x, x-y, y-z and z-u.
 */
class OGRRingAdapter: public OGRAdapterBase<OGRLinearRing>{
private:
    using Base = OGRAdapterBase<OGRLinearRing>;

    /**
     * @brief Wrap this ring into an OGRPolygon
     *
     * To OGR/GEOS an OGRLinearRing is a closed curve (1D). A closed curve has no boundary of its
     * own (the whole ring counts as its interior), so it carries no interior/boundary distinction
     * that area predicates could use. Every area based predicate therefore has to operate on an
     * OGRPolygon wrapping this ring instead.
     */
    OGRUniquePtr<OGRPolygon> toPolygon() const {
        auto polygon = OGRUniquePtr<OGRPolygon>(static_cast<OGRPolygon*>(OGRGeometryFactory::createGeometry(wkbPolygon)));
        polygon->addRing(geomPtr.get());
        return polygon;
    }

    /**
     * @brief Copy this ring into a plain OGRLineString
     *
     * A bare OGRLinearRing is not independently serializable and cannot be handed to GEOS backed
     * operations (see the OGRLinearRing class documentation): passing one either as receiver or as
     * argument silently yields false / -1 instead of a result. Curve level operations
     * (point on boundary, distance, ...) therefore go through an OGRLineString copy.
     */
    OGRLineString toLineString() const {
        OGRLineString line;
        for(int i = 0; i < geomPtr->getNumPoints(); ++i){
            line.addPoint(geomPtr->getX(i), geomPtr->getY(i));
        }
        return line;
    }

    /**
     * @brief Copy an arbitrary IRing into a closed OGRPolygon, ready to be used with GEOS
     */
    static OGRPolygon toPolygon(const IRing auto & ring) {
        OGRLinearRing ogrRing;
        for(const auto & point : ring.getPoints()){
            ogrRing.addPoint(point.getX(), point.getY());
        }
        ogrRing.closeRings();
        OGRPolygon polygon;
        polygon.addRing(&ogrRing);
        return polygon;
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
     * @brief Topological equality, i.e. both geometries cover the very same points
     * @note OGR's own Equals() compares coordinate sequences instead, and would report two
     * rotations of one ring as different.
     */
    static bool topologicallyEquals(const OGRGeometry & lhs, const OGRGeometry & rhs) {
        auto symmetricDifference = OGRUniquePtr<OGRGeometry>(lhs.SymDifference(&rhs));
        return symmetricDifference && symmetricDifference->IsEmpty();
    }

public:
    using numeric_type = double;
    static constexpr GeometryType type = GeometryType::RING;

    OGRRingAdapter(OGRUniquePtr<OGRLinearRing> && ringPtr):Base(std::move(ringPtr)) {
        geomPtr->closeRings();
    }

    OGRRingAdapter(IRing auto const & ring):Base(OGRUniquePtr<OGRLinearRing>(static_cast<OGRLinearRing*>(OGRGeometryFactory::createGeometry(wkbLinearRing)))){
        for(const auto & point : ring.getPoints()){
            geomPtr->addPoint(point.getX(), point.getY());
        }
        geomPtr->closeRings();
    }

    /**
     * @brief Number of points of the open representation, i.e. without the closing point
     * @note an empty ring has no closing point to drop
     */
    int openPointCount() const {
        return std::max(0, geomPtr->getNumPoints() - 1);
    }

    auto getSegments() const -> fishnet::util::forward_range_of<fishnet::geometry::Segment<double>> auto {
        return std::ranges::views::iota(0, openPointCount())
            | std::ranges::views::transform([this](int i) {
                Vec2DReal p1 {geomPtr->getX(i), geomPtr->getY(i)};
                Vec2DReal p2 {geomPtr->getX(i + 1), geomPtr->getY(i + 1)};
                return Segment<double>(p1, p2);
            });
    }

    auto getPoints() const -> fishnet::util::forward_range_of<fishnet::geometry::Vec2DReal> auto {
        return std::ranges::views::iota(0, openPointCount())
            | std::ranges::views::transform([this](int i) {
                return Vec2DReal(geomPtr->getX(i), geomPtr->getY(i));
            });
    }

    const OGRRingAdapter & getBoundary() const{
        return *this;
    }

    std::vector<OGRRingAdapter> getHoles() const{
        return {};
    }

    double area() const{
        return geomPtr->get_Area();
    }

    /**
     * @brief Centroid of the ring, i.e. the mean of its points
     * @note this is the centroid definition used by fishnet::geometry::Ring, not the area weighted
     * centroid OGR/GEOS would compute. The two only coincide for symmetric rings.
     */
    Vec2DReal centroid() const {
        Vec2DReal sum {0,0};
        size_t count = 0;
        for(const auto & point : getPoints()){
            sum = sum + point;
            ++count;
        }
        return sum / static_cast<double>(count);
    }

    Rectangle<double> aaBB() const{
        OGREnvelope boundingBox;
        geomPtr->getEnvelope(&boundingBox);
        return Rectangle<double>(boundingBox.MinX, boundingBox.MaxY, boundingBox.MaxX, boundingBox.MinY);
    }

    bool isInside(const IPoint auto & point) const{
        OGRPoint ogrPoint(point.getX(), point.getY());
        return toPolygon()->Contains(&ogrPoint);
    }

    bool isOnBoundary(const IPoint auto & point) const{
        OGRPoint ogrPoint(point.getX(), point.getY());
        return toLineString().Intersects(&ogrPoint);
    }

    bool isOutside(const IPoint auto & point) const{
        OGRPoint ogrPoint(point.getX(), point.getY());
        return toPolygon()->Disjoint(&ogrPoint);
    }

    bool contains(const IPoint auto & point) const{
        OGRPoint ogrPoint(point.getX(), point.getY());
        return not toPolygon()->Disjoint(&ogrPoint);
    }

    bool contains(const ISegment auto & segment) const{
        OGRLineString ogrLine;
        ogrLine.addPoint(segment.p().getX(), segment.p().getY());
        ogrLine.addPoint(segment.q().getX(), segment.q().getY());
        return covers(*toPolygon(), ogrLine);
    }

    bool contains(const IRing auto & ring) const{
        return covers(*toPolygon(), toPolygon(ring));
    }

    bool contains(const OGRRingAdapter & other) const{
        return covers(*toPolygon(), *other.toPolygon());
    }

    bool intersects(const LinearGeometry auto & linearFeature) const{
        return ringIntersects(*this, linearFeature);
    }

    std::unordered_set<Vec2DReal> intersections(const LinearGeometry auto & linearFeature) const{
        auto intersectionRange = this->getSegments()
            | std::views::transform([&linearFeature](const auto & segment){return segment.intersection(linearFeature);})
            | std::views::filter([](const auto & optIntersection){return optIntersection.has_value();})
            | std::views::transform([](const auto & optIntersection){return optIntersection.value();});
        return std::unordered_set<Vec2D<double>> {std::ranges::begin(intersectionRange), std::ranges::end(intersectionRange)};
    }

    bool crosses(const IRing auto & other) const{
        return std::ranges::any_of(this->getSegments(),[&other](const auto & s){return other.intersects(s);})
            || std::ranges::any_of(other.getSegments(),[this](const auto & s){return this->intersects(s);});
    }

    bool touches(const IRing auto & other) const{
        if(this->crosses(other)) return false;
        if(this->contains(other) || other.contains(*this)) return false;
        for(const auto & p : other.getPoints()){
            if(this->isOnBoundary(p)) return true;
        }
        return false;
    }

    double distance(const IRing auto & other) const{
        if(this->contains(other) or other.contains(*this) or this->crosses(other))
             return -1;
        return shapeDistance(*this,other);
    }

    double distance(const OGRRingAdapter & other) const{
        if(this->contains(other) or other.contains(*this) or this->crosses(other))
             return -1;
        auto thisLine = this->toLineString();
        auto otherLine = other.toLineString();
        return thisLine.Distance(&otherLine);
    }

    /**
     * @brief Equality, independent of the starting vertex and of the winding order
     * (matching fishnet::geometry::Ring)
     */
    bool operator==(const OGRRingAdapter & other) const noexcept {
        if(geomPtr == other.geomPtr)
            return true;
        return topologicallyEquals(*toPolygon(), *other.toPolygon());
    }
};
static_assert(IRing<OGRRingAdapter>);
}
