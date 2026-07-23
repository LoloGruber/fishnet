#pragma once
#include "SweepLine.hpp"
#include "BoundingBoxWrapper.hpp"
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/FixedSizeBuffer.hpp>
#include "PolygonDistance.hpp"
#include "DelaunayTriangulation.hpp"

#include <unordered_map>

namespace fishnet::geometry {

namespace __impl {

/**
 * @brief Type for Polygon Neighbours Sweepline
 * It stores the polygons as BoundingBoxWrappers in the SLS and sorts them from left to right
 * The output is a vector of pairs of polygons of type P, with each pair indicating the adjacency of two polygons
 * Insert events are processed before remove events
 * @tparam P 
 */
template<IPolygon P>
using PolygonNeighbours = SweepLine<BoundingBoxWrapper<P>,std::vector<std::pair<P,P>>,HorizontalAABBOrdering<BoundingBoxWrapper<P>>,true>;

template<IPolygon P>
struct PolygonNeighboursInsertEvent : public PolygonNeighbours<P>::DefaultInsertEvent {
    PolygonNeighboursInsertEvent(const BoundingBoxWrapper<P> & box):PolygonNeighbours<P>::DefaultInsertEvent(box){}
    virtual fishnet::math::DEFAULT_NUMERIC eventPoint() const noexcept {
        return this->obj->getBoundingBox().top();
    }
};

/**
 * @brief Remove Event for Polygon Neighbours Sweepline
 * 
 * @tparam P polygon type
 */
template<IPolygon P>
struct PolygonNeighboursRemoveEvent: public PolygonNeighbours<P>::RemoveEvent {
    util::BiPredicate_t<BoundingBoxWrapper<P>>  neighbouringPredicate; // BiPredicate deciding if two polygons are adjacent
    size_t maxNeighbours;
    PolygonNeighboursRemoveEvent(const BoundingBoxWrapper<P> & bbPptr, util::BiPredicate<BoundingBoxWrapper<P>> auto const & neighbouringBiPredicate,size_t maxNeighbours):PolygonNeighbours<P>::RemoveEvent(bbPptr),neighbouringPredicate(neighbouringBiPredicate),maxNeighbours(maxNeighbours){}

    virtual fishnet::math::DEFAULT_NUMERIC eventPoint() const noexcept {
        return this->obj->getBoundingBox().bottom();
    }

    virtual void process(PolygonNeighbours<P> & sweepLine, std::vector<std::pair<P,P>> & output) const {
        const auto & sls = sweepLine.getSLS();
        const auto & current = *this->obj;
        auto distanceMapper = [&current](const auto & p){
            return shapeDistance(current.getPolygon(),p);
        };
        // auto itInRange = [&current](auto it){
        //     return current.getBoundingBox().left() <= (*it)->getBoundingBox().right() ||
        //         current.getBoundingBox().right() >= (*it)->getBoundingBox().left();
        // };
        auto closestNeighbours = util::FixedSizeBuffer<P,std::invoke_result_t<decltype(distanceMapper),P>>(maxNeighbours,distanceMapper);
        bool skippedSameObject = false; // skip same Polygon object, since it is returned as the lower_bound in the first iteration
        for(auto it = sls.lower_bound(this->obj); it != sls.end(); --it){
            const auto & neighbour = *(*it);
            if(skippedSameObject && neighbouringPredicate(current,neighbour)){
                closestNeighbours.push(neighbour.getPolygon());
                // output.push_back(std::make_pair(current.getPolygon(),neighbour.getPolygon()));   
            }
            skippedSameObject = true;
            if(it == sls.begin())
                break;
        }
        for(auto it = sls.upper_bound(this->obj); it != sls.end();++it){
            const auto & neighbour = *(*it);
            if(neighbouringPredicate(current,neighbour))
                closestNeighbours.push(neighbour.getPolygon());
                // output.push_back(std::make_pair(current.getPolygon(),neighbour.getPolygon()));
        }
        for(auto && neighbour: closestNeighbours) {
            output.emplace_back(current.getPolygon(),std::move(neighbour));
        }
        sweepLine.removeSLS(this->obj);
    }
};

template<PolygonRange R, IPolygon P = std::ranges::range_value_t<R>>
static std::vector<std::pair<P,P>> polygonNeighboursDelaunay(const R & polygons, util::BiPredicate<P> auto const & neighbouringPredicate, double tolerance = 0.0, bool onlyEdges = true) {
    // 1. Extract centroids and build centroid -> polygon map
    std::vector<Vec2DReal> centroids;
    centroids.reserve(util::size(polygons));
    std::unordered_map<Vec2DReal, P, std::hash<Vec2DReal>, std::equal_to<Vec2DReal>> centroidToPolygon;
    for (const auto & polygon : polygons) {
        auto c = polygon.centroid();
        centroids.push_back(c);
        centroidToPolygon.emplace(c, polygon);
    }

    // 2. Delaunay triangulation on centroids
    DelaunayTriangulation delaunay(centroids, tolerance, onlyEdges);

    // 3. Map edges back to polygons and apply predicate
    return delaunay.edges().transform([&](const auto & edges) {
        std::vector<std::pair<P,P>> adjacencies;
        for (const auto & edge : edges) {
            auto it1 = centroidToPolygon.find(edge.p());
            auto it2 = centroidToPolygon.find(edge.q());
            if (it1 != centroidToPolygon.end() && it2 != centroidToPolygon.end() && neighbouringPredicate(it1->second, it2->second)) {
                adjacencies.emplace_back(it1->second, it2->second);
            }
        }
        return adjacencies;
    }).value_or(std::vector<std::pair<P,P>>{});
}

template<PolygonRange R, IPolygon P = std::ranges::range_value_t<R>>
static std::vector<std::pair<P,P>> polygonNeighboursSweep(const R & polygons, util::BiPredicate<BoundingBoxWrapper<P>> auto const & neighbouringPredicate,util::UnaryFunction<P,BoundingBoxWrapper<P>> auto const & wrapper, size_t maxNeighbours) {
    using SweepLine_t = typename __impl::PolygonNeighbours<P>;
    SweepLine_t sweepLine;
    std::vector<std::pair<P,P>> output;
    std::vector<BoundingBoxWrapper<P>> boundingBoxPolygons;
    boundingBoxPolygons.reserve(util::size(polygons));
    std::ranges::for_each(polygons,[&boundingBoxPolygons,&wrapper](const auto & p){
        boundingBoxPolygons.push_back(wrapper(p)); // wrap each polygon in a BoundingBoxWrapper
    });
    std::ranges::for_each(boundingBoxPolygons,[&sweepLine,&neighbouringPredicate,maxNeighbours](const auto & bbPptr){
        sweepLine.addEvent(std::make_unique<__impl::PolygonNeighboursInsertEvent<P>>(bbPptr)); // add insert events to sweepline
        sweepLine.addEvent(std::make_unique<__impl::PolygonNeighboursRemoveEvent<P>>(bbPptr,neighbouringPredicate,maxNeighbours)); // add remove events to sweepline
    });
    return sweepLine.sweep(output);
}

} // namespace __impl

struct PolygonNeighbours{
public:
    
/**
 * @brief Find neighbouring polygons using Delaunay triangulation on centroids.
 *
 * Computes the centroid for each polygon, builds a Delaunay triangulation,
 * and returns an edge for every triangle side connecting two centroids.
 * A BiPredicate filters which edges are considered valid neighbours.
 *
 * @tparam R range type
 * @tparam P polygon type == value type of range
 * @param polygons range of polygons
 * @param neighbouringPredicate BiPredicate deciding whether two Polygons are neighbours
 * @return std::vector<std::pair<P,P>> list of pairs, indicating the neighbouring relationship of two polygons
 */
template<PolygonRange R, IPolygon P = std::ranges::range_value_t<R>>
static std::vector<std::pair<P,P>> delaunay(const R & polygons, util::BiPredicate<P> auto const & neighbouringPredicate, double tolerance = 0.0, bool onlyEdges = true) {
    return __impl::polygonNeighboursDelaunay(polygons,neighbouringPredicate, tolerance, onlyEdges);
}

/**
 * @brief Overload without predicate — returns all Delaunay edges.
 * @param polygons range of polygons
 */
template<PolygonRange R, IPolygon P = std::ranges::range_value_t<R>>
static std::vector<std::pair<P,P>> delaunay(const R & polygons) {
    return delaunay(polygons, fishnet::util::TrueBiPredicate{});
}

/**
 * @brief Generic findNeighbouringPolygons function, which returns a list of pairs indicating the adjacencies of the polygons
 * 
 * @tparam R range type
 * @tparam P polygon type == value type of range
 * @param polygons range of polygons
 * @param neighbouringPredicate BiPredicate deciding whether two BoundingBoxWrappers are neighbours
 * @param wrapper unary function which wraps polygons of type P into BoundingBoxWrappers required for the sweepline
 * @return std::vector<std::pair<P,P>> list of pairs, indicating the neighbouring relationship of the polygons
 */
template<PolygonRange R, IPolygon P = std::ranges::range_value_t<R>>
static std::vector<std::pair<P,P>> sweepTemplate(const R & polygons, util::BiPredicate<BoundingBoxWrapper<P>> auto const & neighbouringPredicate,util::UnaryFunction<P,BoundingBoxWrapper<P>> auto const & wrapper, size_t maxNeighbours) {
    return __impl::polygonNeighboursSweep(polygons,neighbouringPredicate,wrapper,maxNeighbours);
}

/**
 * @brief Finding neighbours of polygons using a sweepline, returns a list of pairs indicating the adjacencies of two polygons
 * 
 * @tparam R range type
 * @tparam P polygon type == value type of range
 * @param polygons range of polygons
 * @param neighbouringPredicate BiPredicate deciding whether two Polygons of type P are neighbours
 * @param wrapper unary function which wraps polygons of type P into BoundingBoxWrappers required for the sweepline
 * @return std::vector<std::pair<P,P>> list of pairs, indicating the neighbouring relationship of two polygons
 */
template<PolygonRange R, IPolygon P = std::ranges::range_value_t<R>>
static std::vector<std::pair<P,P>> sweep(const R & polygons, util::BiPredicate<P> auto  && neighbouringPredicate,util::UnaryFunction<P,BoundingBoxWrapper<P>> auto const & wrapper,size_t maxNeighbours) {
    return sweepTemplate(polygons, [&neighbouringPredicate](const BoundingBoxWrapper<P> & current, const BoundingBoxWrapper<P> & neighbour){
        return neighbouringPredicate(current.getPolygon(),neighbour.getPolygon());
    },wrapper,maxNeighbours);
}

/**
 * @brief Overload with wrapper into default BoundingBoxWrappers (no custom bounding box allowed)
 * 
 * @tparam R 
 * @tparam P 
 * @param polygons 
 * @param neighbouringPredicate BiPredicate deciding whether two Polygons of type P are neighbours
 * @return std::vector<std::pair<P,P>> list of pairs, indicating the neighbouring relationship of two polygons
 */
template<PolygonRange R, IPolygon P = std::ranges::range_value_t<R>>
static std::vector<std::pair<P,P>> sweep(const R & polygons, util::BiPredicate<P> auto const & neighbouringPredicate,size_t maxNeighbours) {
    return sweep(polygons,neighbouringPredicate,[](const P & p){return BoundingBoxWrapper(p);},maxNeighbours);
}

/**
 * @brief Overload for finding neighbouring polygons with a custom buffer multiplier for the bounding boxes
 * 
 * @tparam R 
 * @param polygons 
 * @param bufferMultiplier 
 * @return std::vector<std::pair<P,P>> list of pairs, indicating the neighbouring relationship of two polygons  
 */
template<PolygonRange R>
static std::vector<std::pair<std::ranges::range_value_t<R>,std::ranges::range_value_t<R>>> sweepWithFixedBuffer(const R & polygons, fishnet::math::DEFAULT_NUMERIC bufferMultiplier, size_t maxNeighbours) {
    if (bufferMultiplier <= 1)
        throw std::invalid_argument("Buffer range multiplier has to be greater than 1");
    using P = std::ranges::range_value_t<R>;

    auto crossesOrContainedInBoundingBox = [](const BoundingBoxWrapper<P> & current,const BoundingBoxWrapper<P> & neighbour){
            return neighbour.getBoundingBox().crosses(current.getBoundingBox()) || neighbour.getBoundingBox().contains(current.getBoundingBox()) || current.getBoundingBox().contains(neighbour.getBoundingBox());
    }; // polygons are in relation if (scaled) bounding boxes overlap

    auto scaledWrapper = [bufferMultiplier](const P & polygon) {
        auto aaBBRectangle = Rectangle<fishnet::math::DEFAULT_NUMERIC>(polygon);
        return BoundingBoxWrapper(polygon,aaBBRectangle.scale(bufferMultiplier));
    };
    return sweep(polygons, crossesOrContainedInBoundingBox,scaledWrapper,maxNeighbours);
}
};
} // namespace fishnet::geometry
