#pragma once
#include "SweepLine.hpp"
#include "BoundingBoxPolygon.hpp"
#include <fishnet/FunctionalConcepts.hpp>

namespace fishnet::geometry {

namespace __impl {

template<IPolygon P>
using PolygonNeighbours = SweepLine<BoundingBoxPolygon<P>,std::pair<P,P>,HorizontalAABBOrdering<P>>;

template<IPolygon P>
struct PolygonNeighboursInsertEvent : public PolygonNeighbours<P>::DefaultInsertEvent {
    PolygonNeighboursInsertEvent(const BoundingBoxPolygon<P> & box):PolygonNeighbours<P>::DefaultInsertEvent(box){}
    virtual fishnet::math::DEFAULT_NUMERIC eventPoint() const noexcept {
        return this->obj->getBoundingBox().top();
    }


};

template<IPolygon P>
struct PolygonNeighboursRemoveEvent: public PolygonNeighbours<P>::RemoveEvent {
    util::BiPredicate_t<BoundingBoxPolygon<P>>  neighbouringPredicate;
    PolygonNeighboursRemoveEvent(const BoundingBoxPolygon<P> & bbPptr, util::BiPredicate<BoundingBoxPolygon<P>> auto const & neighbouringBiPredicate):PolygonNeighbours<P>::RemoveEvent(bbPptr),neighbouringPredicate(neighbouringBiPredicate){}

    virtual fishnet::math::DEFAULT_NUMERIC eventPoint() const noexcept {
        return this->obj->getBoundingBox().bottom();
    }

    virtual void process(PolygonNeighbours<P> & sweepLine, std::vector<std::pair<P,P>> & output) const {
        const auto & sls = sweepLine.getSLS();
        const auto & current = *this->obj;

        bool skippedSameObject = false;
        for(auto it = sls.lower_bound(this->obj); it != sls.begin() && it != sls.end(); --it){
            const auto & neighbour = *(*it);
            if(skippedSameObject && neighbouringPredicate(current,neighbour)){
                output.push_back(std::make_pair(current.getPolygon(),neighbour.getPolygon()));   
            }
            skippedSameObject = true;
        }
        for(auto it = sls.upper_bound(this->obj); it != sls.end();++it){
            const auto & neighbour = *(*it);
            if(neighbouringPredicate(current,neighbour))
                output.push_back(std::make_pair(current.getPolygon(),neighbour.getPolygon()));
        }
        sweepLine.removeSLS(this->obj);
    }
};
template<PolygonRange R, IPolygon P = std::ranges::range_value_t<R>>
static std::vector<std::pair<P,P>> findNeighbouringPolygons(const R & polygons, util::BiPredicate<BoundingBoxPolygon<P>> auto const & neighbouringPredicate,util::UnaryFunction<P,BoundingBoxPolygon<P>> auto const & wrapper) {
    using SweepLine_t = typename __impl::PolygonNeighbours<P>;
    SweepLine_t sweepLine;
    std::vector<std::pair<P,P>> output;
    std::vector<BoundingBoxPolygon<P>> boundingBoxPolygons;
    boundingBoxPolygons.reserve(util::size(polygons));
    std::ranges::for_each(polygons,[&boundingBoxPolygons,&wrapper](const auto & p){
        boundingBoxPolygons.push_back(wrapper(p));
    });
    std::ranges::for_each(boundingBoxPolygons,[&sweepLine,&neighbouringPredicate](const auto & bbPptr){
        sweepLine.addEvent(std::make_unique<__impl::PolygonNeighboursInsertEvent<P>>(bbPptr));
        sweepLine.addEvent(std::make_unique<__impl::PolygonNeighboursRemoveEvent<P>>(bbPptr,neighbouringPredicate));
    });
    return sweepLine.sweep(output);
}
}



template<PolygonRange R, IPolygon P = std::ranges::range_value_t<R>>
static std::vector<std::pair<P,P>> findNeighbouringPolygons(const R & polygons, util::BiPredicate<P> auto const & neighbouringPredicate,util::UnaryFunction<P,BoundingBoxPolygon<P>> auto const & wrapper) {
    return __impl::findNeighbouringPolygons(polygons, [&neighbouringPredicate](const BoundingBoxPolygon<P> & current, const BoundingBoxPolygon<P> & neighbour){
        return neighbouringPredicate(current.getPolygon(),neighbour.getPolygon());
    },wrapper);
}


template<PolygonRange R, IPolygon P = std::ranges::range_value_t<R>>
static std::vector<std::pair<P,P>> findNeighbouringPolygons(const R & polygons, util::BiPredicate<P> auto const & neighbouringPredicate) {
    return findNeighbouringPolygons(polygons,neighbouringPredicate,[](const P & p){return BoundingBoxPolygon(p);});
}

template<PolygonRange R>
static std::vector<std::pair<std::ranges::range_value_t<R>,std::ranges::range_value_t<R>>> nearestPolygonNeighbours(const R & polygons, fishnet::math::DEFAULT_NUMERIC bufferMultiplier) {
    if (bufferMultiplier <= 1)
        throw std::invalid_argument("Buffer range multiplier has to be greater than 1");
    using P = std::ranges::range_value_t<R>;

    auto crossesOrContainedInBoundingBox = [](const BoundingBoxPolygon<P> & current,const BoundingBoxPolygon<P> & neighbour){
            return neighbour.getBoundingBox().crosses(current.getBoundingBox()) || neighbour.getBoundingBox().contains(current.getBoundingBox()) || current.getBoundingBox().contains(neighbour.getBoundingBox());
    };

    auto scaledWrapper = [bufferMultiplier](const P & polygon) {
        auto aaBBRectangle = Rectangle<fishnet::math::DEFAULT_NUMERIC>(polygon.aaBB().getPoints());
        return BoundingBoxPolygon(polygon,aaBBRectangle.scale(bufferMultiplier));
    };
    return __impl::findNeighbouringPolygons(polygons, crossesOrContainedInBoundingBox,scaledWrapper);
}
}
