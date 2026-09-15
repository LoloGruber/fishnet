#pragma once
#include <numeric>
#include <fishnet/IGeometry.hpp>
#include <fishnet/Rectangle.hpp>
#include <fishnet/PolygonDistance.hpp>
#include "OGRAdapterBase.hpp"
#include "OGRPolygonAdapter.hpp"

namespace fishnet::geometry{

/**
 * @brief IMultiPolygon implementation backed by an OGRMultiPolygon
 *
 * Mirrors the semantics of fishnet::geometry::MultiPolygon: the polygons of a multi-polygon must
 * neither be duplicates, nor cross, nor contain one another, which is enforced by addPolygon().
 */
class OGRMultiPolygonAdapter: public OGRAdapterBase<OGRMultiPolygon>{
private:
    using Base = OGRAdapterBase<OGRMultiPolygon>;

    static OGRUniquePtr<OGRMultiPolygon> emptyMultiPolygon() {
        return OGRUniquePtr<OGRMultiPolygon>(static_cast<OGRMultiPolygon*>(OGRGeometryFactory::createGeometry(wkbMultiPolygon)));
    }

    static bool covers(const OGRGeometry & covering, const OGRGeometry & covered) {
        auto difference = OGRUniquePtr<OGRGeometry>(covered.Difference(&covering));
        return difference && difference->IsEmpty();
    }

    OGRPolygon * polygonAt(int index) const {
        return static_cast<OGRPolygon*>(geomPtr->getGeometryRef(index));
    }

public:
    using numeric_type = double;
    using polygon_type = OGRPolygonAdapter;
    static constexpr GeometryType type = GeometryType::MULTIPOLYGON;

    OGRMultiPolygonAdapter(OGRUniquePtr<OGRMultiPolygon> && multiPolygonPtr):Base(std::move(multiPolygonPtr)) {
        geomPtr->closeRings();
    }

    OGRMultiPolygonAdapter(const OGRPolygonAdapter & polygon):Base(emptyMultiPolygon()){
        geomPtr->addGeometry(polygon.raw());
    }

    OGRMultiPolygonAdapter(IMultiPolygon auto const & multiPolygon):Base(emptyMultiPolygon()){
        for(const auto & polygon : multiPolygon.getPolygons()){
            OGRPolygonAdapter adapted {polygon};
            geomPtr->addGeometry(adapted.raw());
        }
    }

    /**
     * @brief Construct a multi-polygon from a range of polygons
     * @param polygons range of IPolygon objects
     * @param checked if true the polygons are added as they are, otherwise invalid entries
     * (duplicates, crossing or contained polygons) are skipped
     */
    OGRMultiPolygonAdapter(const PolygonRange auto & polygons, bool checked = false):Base(emptyMultiPolygon()){
        for(const auto & polygon : polygons){
            OGRPolygonAdapter adapted {polygon};
            if(checked)
                geomPtr->addGeometry(adapted.raw());
            else
                addPolygon(adapted);
        }
    }

    size_t size() const noexcept {
        return static_cast<size_t>(geomPtr->getNumGeometries());
    }

    auto getPolygons() const -> fishnet::util::forward_range_of<OGRPolygonAdapter> auto {
        return std::ranges::views::iota(0, geomPtr->getNumGeometries())
            | std::ranges::views::transform([this](int i){
                return OGRPolygonAdapter(OGRUniquePtr<OGRPolygon>(polygonAt(i)->clone()));
            });
    }

    /**
     * @brief Add a polygon to the multi-polygon
     * @return false if the polygon is a duplicate, crosses or is contained in (or contains) one of
     * the polygons already present, true on success
     */
    bool addPolygon(const OGRPolygonAdapter & polygon) noexcept {
        for(const auto & present : getPolygons()){
            if(present == polygon)
                return false;
            if(present.crosses(polygon))
                return false;
            if(present.contains(polygon) || polygon.contains(present))
                return false;
        }
        return geomPtr->addGeometry(polygon.raw()) == OGRERR_NONE;
    }

    bool removePolygon(const OGRPolygonAdapter & polygon) noexcept {
        for(int i = 0; i < geomPtr->getNumGeometries(); ++i){
            if(polygonAt(i)->Equals(polygon.raw())){
                return geomPtr->removeGeometry(i) == OGRERR_NONE;
            }
        }
        return false;
    }

    double area() const {
        return geomPtr->get_Area();
    }

    /**
     * @brief Centroid of the multi-polygon, i.e. the centroids of its polygons weighted by area
     */
    Vec2DReal centroid() const {
        Vec2DReal accumulatedWeightedCentroid {0.0,0.0};
        double totalArea = this->area();
        for(const auto & polygon : getPolygons()){
            accumulatedWeightedCentroid = accumulatedWeightedCentroid + polygon.centroid() * (polygon.area() / totalArea);
        }
        return accumulatedWeightedCentroid;
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

    bool isOnBoundary(const IPoint auto & point) const {
        OGRPoint ogrPoint(point.getX(), point.getY());
        return geomPtr->Touches(&ogrPoint);
    }

    bool isOutside(const IPoint auto & point) const {
        OGRPoint ogrPoint(point.getX(), point.getY());
        return geomPtr->Disjoint(&ogrPoint);
    }

    bool contains(const IPoint auto & point) const {
        OGRPoint ogrPoint(point.getX(), point.getY());
        return not geomPtr->Disjoint(&ogrPoint);
    }

    bool contains(const ISegment auto & segment) const {
        // a segment is contained if it is contained in one of the polygons, being spread over
        // multiple polygons of the multi-polygon does not count
        return std::ranges::any_of(getPolygons(),[&segment](const auto & polygon){return polygon.contains(segment);});
    }

    bool contains(const IPolygon auto & query) const {
        return std::ranges::any_of(getPolygons(),[&query](const auto & polygon){return polygon.contains(query);});
    }

    bool containsInHole(const IPolygon auto & query) const {
        return std::ranges::any_of(getPolygons(),[&query](const auto & polygon){return polygon.containsInHole(query);});
    }

    bool intersects(const LinearGeometry auto & linearFeature) const {
        return std::ranges::any_of(getPolygons(),[&linearFeature](const auto & polygon){return polygon.intersects(linearFeature);});
    }

    std::unordered_set<Vec2DReal> intersections(const LinearGeometry auto & linearFeature) const {
        std::unordered_set<Vec2DReal> intersectionSet;
        for(const auto & polygon : getPolygons()){
            for(const auto & point : polygon.intersections(linearFeature)){
                intersectionSet.insert(point);
            }
        }
        return intersectionSet;
    }

    bool crosses(const IPolygon auto & query) const {
        return std::ranges::any_of(getPolygons(),[&query](const auto & polygon){return polygon.crosses(query);});
    }

    bool touches(const IPolygon auto & query) const {
        return not contains(query) && not crosses(query)
            && std::ranges::any_of(getPolygons(),[&query](const auto & polygon){return polygon.touches(query);});
    }

    double distance(const IPolygon auto & query) const {
        return std::ranges::min(getPolygons() | std::views::transform([&query](const auto & polygon){return polygon.distance(query);}));
    }

    double distance(const IMultiPolygon auto & other) const {
        return std::ranges::min(other.getPolygons() | std::views::transform([this](const auto & otherPolygon){return this->distance(otherPolygon);}));
    }

    /**
     * @brief Equality, independent of the order of the polygons (matching
     * fishnet::geometry::MultiPolygon)
     * @note OGR's own Equals() compares the members pairwise in order instead.
     */
    bool operator==(const OGRMultiPolygonAdapter & other) const noexcept {
        if(geomPtr == other.geomPtr)
            return true;
        auto symmetricDifference = OGRUniquePtr<OGRGeometry>(geomPtr->SymDifference(other.geomPtr.get()));
        return symmetricDifference && symmetricDifference->IsEmpty();
    }

    /**
     * @brief Hash compatible with fishnet::geometry::MultiPolygon, i.e. independent of the order
     * of the polygons
     */
    size_t hash() const noexcept {
        auto polygonHashes = getPolygons() | std::views::transform([](const auto & polygon){return polygon.hash();});
        return std::accumulate(std::ranges::begin(polygonHashes), std::ranges::end(polygonHashes), size_t(0));
    }
};
static_assert(IMultiPolygon<OGRMultiPolygonAdapter>);
static_assert(Shape<OGRMultiPolygonAdapter>);
}
