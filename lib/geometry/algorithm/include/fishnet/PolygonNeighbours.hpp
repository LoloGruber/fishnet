#pragma once
#include "SweepLine.hpp"
#include "BoundingBoxPolygon.hpp"
#include <fishnet/FunctionalConcepts.hpp>
#include <fishnet/FixedSizePriorityQueue.hpp>

namespace fishnet::geometry {

namespace __impl {

/**
 * @brief Type for Polygon Neighbours Sweepline
 * It stores the polygons as BoundingBoxPolygons in the SLS and sorts them from left to right
 * The output is a vector of pairs of polygons of type P, with each pair indicating the adjacency of two polygons
 * Insert events are processed before remove events
 * @tparam P 
 */
template<IPolygon P>
using PolygonNeighbours = SweepLine<BoundingBoxPolygon<P>,std::pair<P,P>,HorizontalAABBOrdering<P>,true>;

template<IPolygon P>
struct PolygonNeighboursInsertEvent : public PolygonNeighbours<P>::DefaultInsertEvent {
    PolygonNeighboursInsertEvent(const BoundingBoxPolygon<P> & box):PolygonNeighbours<P>::DefaultInsertEvent(box){}
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
    util::BiPredicate_t<BoundingBoxPolygon<P>>  neighbouringPredicate; // BiPredicate deciding if two polygons are adjacent
    size_t k;
    PolygonNeighboursRemoveEvent(const BoundingBoxPolygon<P> & bbPptr, util::BiPredicate<BoundingBoxPolygon<P>> auto const & neighbouringBiPredicate,size_t k):PolygonNeighbours<P>::RemoveEvent(bbPptr),neighbouringPredicate(neighbouringBiPredicate),k(k){}

    virtual fishnet::math::DEFAULT_NUMERIC eventPoint() const noexcept {
        return this->obj->getBoundingBox().bottom();
    }

    virtual void process(PolygonNeighbours<P> & sweepLine, std::vector<std::pair<P,P>> & output) const {
        const auto & sls = sweepLine.getSLS();
        const auto & current = *this->obj;
        auto distanceMapper = [&current](const auto & p){
            return current.getPolygon().distance(p);
        };
        // auto itInRange = [&current](auto it){
        //     return current.getBoundingBox().left() <= (*it)->getBoundingBox().right() ||
        //         current.getBoundingBox().right() >= (*it)->getBoundingBox().left();
        // };
        auto closestNeighbours = util::FixedSizePriorityQueue<P,decltype(distanceMapper)>(k,distanceMapper);
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

/**
 * @brief Generic findNeighbouringPolygons function, which returns a list of pairs indicating the adjacencies of two polygons
 * 
 * @tparam R range type
 * @tparam P polygon type == value type of range
 * @param polygons range of polygons
 * @param neighbouringPredicate BiPredicate deciding whether two BoundingBoxPolygons are neighbours
 * @param wrapper unary function which wraps polygons of type P into BoundingBoxPolygons required for the sweepline
 * @return std::vector<std::pair<P,P>> list of pairs, indicating the neighbouring relationship of two polygons
 */
template<PolygonRange R, IPolygon P = std::ranges::range_value_t<R>>
static std::vector<std::pair<P,P>> findNeighbouringPolygons(const R & polygons, util::BiPredicate<BoundingBoxPolygon<P>> auto const & neighbouringPredicate,util::UnaryFunction<P,BoundingBoxPolygon<P>> auto const & wrapper, size_t k) {
    using SweepLine_t = typename __impl::PolygonNeighbours<P>;
    SweepLine_t sweepLine;
    std::vector<std::pair<P,P>> output;
    std::vector<BoundingBoxPolygon<P>> boundingBoxPolygons;
    boundingBoxPolygons.reserve(util::size(polygons));
    std::ranges::for_each(polygons,[&boundingBoxPolygons,&wrapper](const auto & p){
        boundingBoxPolygons.push_back(wrapper(p)); // wrap each polygon in a BoundingBoxPolygon
    });
    std::ranges::for_each(boundingBoxPolygons,[&sweepLine,&neighbouringPredicate,k](const auto & bbPptr){
        sweepLine.addEvent(std::make_unique<__impl::PolygonNeighboursInsertEvent<P>>(bbPptr)); // add insert events to sweepline
        sweepLine.addEvent(std::make_unique<__impl::PolygonNeighboursRemoveEvent<P>>(bbPptr,neighbouringPredicate,k)); // add remove events to sweepline
    });
    return sweepLine.sweep(output);
}
}


/**
 * @brief Finding neighbours of polygons using a sweepline, returns a list of pairs indicating the adjacencies of two polygons
 * 
 * @tparam R range type
 * @tparam P polygon type == value type of range
 * @param polygons range of polygons
 * @param neighbouringPredicate BiPredicate deciding whether two Polygons of type P are neighbours
 * @param wrapper unary function which wraps polygons of type P into BoundingBoxPolygons required for the sweepline
 * @return std::vector<std::pair<P,P>> list of pairs, indicating the neighbouring relationship of two polygons
 */
template<PolygonRange R, IPolygon P = std::ranges::range_value_t<R>>
static std::vector<std::pair<P,P>> findNeighbouringPolygons(const R & polygons, util::BiPredicate<P> auto  && neighbouringPredicate,util::UnaryFunction<P,BoundingBoxPolygon<P>> auto const & wrapper,size_t k) {
    return __impl::findNeighbouringPolygons(polygons, [&neighbouringPredicate](const BoundingBoxPolygon<P> & current, const BoundingBoxPolygon<P> & neighbour){
        return neighbouringPredicate(current.getPolygon(),neighbour.getPolygon());
    },wrapper,k);
}

/**
 * @brief Overload with wrapper into default BoundingBoxPolygons (no custom bounding box allowed)
 * 
 * @tparam R 
 * @tparam P 
 * @param polygons 
 * @param neighbouringPredicate BiPredicate deciding whether two Polygons of type P are neighbours
 * @return std::vector<std::pair<P,P>> list of pairs, indicating the neighbouring relationship of two polygons
 */
template<PolygonRange R, IPolygon P = std::ranges::range_value_t<R>>
static std::vector<std::pair<P,P>> findNeighbouringPolygons(const R & polygons, util::BiPredicate<P> auto const & neighbouringPredicate,size_t k) {
    return findNeighbouringPolygons(polygons,neighbouringPredicate,[](const P & p){return BoundingBoxPolygon(p);},k);
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
static std::vector<std::pair<std::ranges::range_value_t<R>,std::ranges::range_value_t<R>>> findNeighbouringPolygons(const R & polygons, fishnet::math::DEFAULT_NUMERIC bufferMultiplier) {
    if (bufferMultiplier <= 1)
        throw std::invalid_argument("Buffer range multiplier has to be greater than 1");
    using P = std::ranges::range_value_t<R>;

    auto crossesOrContainedInBoundingBox = [](const BoundingBoxPolygon<P> & current,const BoundingBoxPolygon<P> & neighbour){
            return neighbour.getBoundingBox().crosses(current.getBoundingBox()) || neighbour.getBoundingBox().contains(current.getBoundingBox()) || current.getBoundingBox().contains(neighbour.getBoundingBox());
    }; // polygons are in relation if (scaled) bounding boxes overlap

    auto scaledWrapper = [bufferMultiplier](const P & polygon) {
        auto aaBBRectangle = Rectangle<fishnet::math::DEFAULT_NUMERIC>(polygon.aaBB().getPoints());
        return BoundingBoxPolygon(polygon,aaBBRectangle.scale(bufferMultiplier));
    };
    return __impl::findNeighbouringPolygons(polygons, crossesOrContainedInBoundingBox,scaledWrapper);
}
}
