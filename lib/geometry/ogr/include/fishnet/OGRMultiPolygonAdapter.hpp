#pragma once
#include <numeric>
#include <fishnet/IGeometry.hpp>
#include <fishnet/Rectangle.hpp>
#include <fishnet/MultiPolygon.hpp>
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

    /**
     * @brief Bring the wrapped multi-polygon into the canonical form of GEOS
     *
     * Canonicalising keeps the inherited coordinate wise operator== independent of the order of
     * the polygons and of the starting vertex of each ring, and keeps it consistent with the
     * inherited WKB hash. It has to be repeated after every mutation of the collection.
     */
    void canonicalize() noexcept {
        auto normalized = OGRUniquePtr<OGRGeometry>(geomPtr->Normalize());
        if(normalized && wkbFlatten(normalized->getGeometryType()) == wkbMultiPolygon){
            geomPtr.reset(normalized.release()->toMultiPolygon());
        }
    }

    OGRPolygon * polygonAt(int index) const {
        return static_cast<OGRPolygon*>(geomPtr->getGeometryRef(index));
    }

    /**
     * @brief The members of the collection, wrapped without being copied
     *
     * Everything below only reads the members while this multi-polygon is alive, so they can be
     * borrowed rather than cloned. The public getPolygons() hands out owning copies instead,
     * because those may outlive this object.
     * @warning the elements must not be stored beyond the traversal
     */
    auto borrowedPolygons() const -> fishnet::util::forward_range_of<OGRPolygonAdapter> auto {
        return std::ranges::views::iota(0, geomPtr->getNumGeometries())
            | std::ranges::views::transform([this](int i){
                return OGRPolygonAdapter::borrowed(polygonAt(i));
            });
    }

    /**
     * @brief Whether a polygon may join the collection, i.e. it is no duplicate of a polygon
     * already present, does not cross one and neither contains nor is contained by one
     */
    bool isAddable(const OGRPolygonAdapter & polygon) const {
        for(const auto & present : borrowedPolygons()){
            if(present == polygon)
                return false;
            if(present.crosses(polygon))
                return false;
            if(present.contains(polygon) || polygon.contains(present))
                return false;
        }
        return true;
    }

    friend class OGRGeometryAdapter;

    /**
     * @brief Hand the wrapped multi-polygon over as a standalone geometry, without copying it
     * @note the multi-polygon is left moved-from and must not be used again
     */
    OGRUniquePtr<OGRGeometry> releaseGeometry() && noexcept {
        auto deleter = geomPtr.get_deleter();
        return OGRUniquePtr<OGRGeometry>(geomPtr.release(), deleter);
    }
public:
    using numeric_type = double;
    using polygon_type = OGRPolygonAdapter;
    static constexpr GeometryType type = GeometryType::MULTIPOLYGON;

    OGRMultiPolygonAdapter(OGRUniquePtr<OGRMultiPolygon> && multiPolygonPtr):Base(std::move(multiPolygonPtr)) {
        geomPtr->closeRings();
        canonicalize();
    }

    OGRMultiPolygonAdapter(IMultiPolygon auto const & multiPolygon):Base(emptyMultiPolygon()){
        for(const auto & polygon : multiPolygon.getPolygons()){
            OGRPolygonAdapter adapted {polygon};
            geomPtr->addGeometry(adapted.raw());
        }
        canonicalize();
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
            // every polygon is canonical on its own, so only the order of the members is left to
            // canonicalise, once, after all of them have been added
            if(checked || isAddable(adapted))
                geomPtr->addGeometry(adapted.raw());
        }
        canonicalize();
    }

    size_t size() const noexcept {
        return static_cast<size_t>(geomPtr->getNumGeometries());
    }

    /**
     * @note every member of the wrapped multi-polygon is canonical (@see canonicalize()), so the
     * members are handed out without normalising them again
     */
    auto getPolygons() const -> fishnet::util::forward_range_of<OGRPolygonAdapter> auto {
        return std::ranges::views::iota(0, geomPtr->getNumGeometries())
            | std::ranges::views::transform([this](int i){
                return OGRPolygonAdapter::fromCanonicalPolygon(*polygonAt(i));
            });
    }

    /**
     * @brief Add a polygon to the multi-polygon
     * @return false if the polygon is a duplicate, crosses or is contained in (or contains) one of
     * the polygons already present, true on success
     */
    bool addPolygon(const OGRPolygonAdapter & polygon) noexcept {
        if(not isAddable(polygon))
            return false;
        if(geomPtr->addGeometry(polygon.raw()) != OGRERR_NONE)
            return false;
        canonicalize();
        return true;
    }

    bool removePolygon(const OGRPolygonAdapter & polygon) noexcept {
        for(int i = 0; i < geomPtr->getNumGeometries(); ++i){
            if(polygonAt(i)->Equals(polygon.raw())){
                if(geomPtr->removeGeometry(i) != OGRERR_NONE)
                    return false;
                canonicalize();
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Copy the multi-polygon out into a fishnet MultiPolygon, which precomputes its segments
     * @see OGRRingAdapter::toNative()
     */
    MultiPolygon<Polygon<double>> toNative() const {
        std::vector<Polygon<double>> polygons;
        polygons.reserve(this->size());
        for(const auto & polygon : borrowedPolygons()){
            polygons.push_back(polygon.toNative());
        }
        return MultiPolygon<Polygon<double>>(polygons, true);
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
        for(const auto & polygon : borrowedPolygons()){
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
        return std::ranges::any_of(borrowedPolygons(),[&segment](const auto & polygon){return polygon.contains(segment);});
    }

    bool contains(const IPolygon auto & query) const {
        return std::ranges::any_of(borrowedPolygons(),[&query](const auto & polygon){return polygon.contains(query);});
    }

    bool containsInHole(const IPolygon auto & query) const {
        return std::ranges::any_of(borrowedPolygons(),[&query](const auto & polygon){return polygon.containsInHole(query);});
    }

    bool intersects(const LinearGeometry auto & linearFeature) const {
        return std::ranges::any_of(borrowedPolygons(),[&linearFeature](const auto & polygon){return polygon.intersects(linearFeature);});
    }

    std::unordered_set<Vec2DReal> intersections(const LinearGeometry auto & linearFeature) const {
        std::unordered_set<Vec2DReal> intersectionSet;
        for(const auto & polygon : borrowedPolygons()){
            for(const auto & point : polygon.intersections(linearFeature)){
                intersectionSet.insert(point);
            }
        }
        return intersectionSet;
    }

    bool crosses(const IPolygon auto & query) const {
        return std::ranges::any_of(borrowedPolygons(),[&query](const auto & polygon){return polygon.crosses(query);});
    }

    bool touches(const IPolygon auto & query) const {
        return not contains(query) && not crosses(query)
            && std::ranges::any_of(borrowedPolygons(),[&query](const auto & polygon){return polygon.touches(query);});
    }

    double distance(const IPolygon auto & query) const {
        return std::ranges::min(borrowedPolygons() | std::views::transform([&query](const auto & polygon){return polygon.distance(query);}));
    }

    double distance(const IMultiPolygon auto & other) const {
        return std::ranges::min(other.getPolygons() | std::views::transform([this](const auto & otherPolygon){return this->distance(otherPolygon);}));
    }

    /**
     * @brief Distance between two OGR backed multi-polygons
     * @note mirrors the generic overloads above (0 if any pair of member polygons overlaps or
     * touches, otherwise the smallest gap between any pair), but leaves finding that pair to GEOS
     * instead of checking every polygon of this against every polygon of other by hand
     */
    double distance(const OGRMultiPolygonAdapter & other) const {
        return geomPtr->Distance(other.geomPtr.get());
    }
};
static_assert(IMultiPolygon<OGRMultiPolygonAdapter>);
static_assert(Shape<OGRMultiPolygonAdapter>);
}
