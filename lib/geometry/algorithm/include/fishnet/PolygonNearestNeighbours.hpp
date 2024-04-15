#pragma once
#include "SweepLine.hpp"
#include "BoundingBoxPolygon.hpp"

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
    PolygonNeighboursRemoveEvent(const BoundingBoxPolygon<P> & bbPptr):PolygonNeighbours<P>::RemoveEvent(bbPptr){}
    virtual fishnet::math::DEFAULT_NUMERIC eventPoint() const noexcept {
        return this->obj->getBoundingBox().bottom();
    }

    virtual void process(PolygonNeighbours<P> & sweepLine, std::vector<std::pair<P,P>> & output) const {
        const auto & sls = sweepLine.getSLS();
        const auto & boundingBoxPolygon = this->obj;
        auto crossesOrContainedInBoundingBox = [&boundingBoxPolygon](const BoundingBoxPolygon<P> * bbPptr){
            return bbPptr->getBoundingBox().crosses(boundingBoxPolygon->getBoundingBox()) || bbPptr->getBoundingBox().contains(boundingBoxPolygon->getBoundingBox()) || boundingBoxPolygon->getBoundingBox().contains(bbPptr->getBoundingBox());
        };
        bool skipSame = false;
        for(auto it = sls.lower_bound(this->obj); it != sls.begin() && it != sls.end() /* && crossesOrContainedInBoundingBox(*it) */; --it){
            if(skipSame && crossesOrContainedInBoundingBox(*it))
                output.push_back(std::make_pair(boundingBoxPolygon->getPolygon(),(*it)->getPolygon()));
            skipSame = true;
        }
        for(auto it = sls.upper_bound(this->obj); it != sls.end() /* && crossesOrContainedInBoundingBox(*it) */;++it){
            if(crossesOrContainedInBoundingBox(*it))
                output.push_back(std::make_pair(boundingBoxPolygon->getPolygon(),(*it)->getPolygon()));
        }
        sweepLine.removeSLS(this->obj);
    }
};
}

template<PolygonRange R>
static std::vector<std::pair<std::ranges::range_value_t<R>,std::ranges::range_value_t<R>>> nearestPolygonNeighbours(const R & polygons, fishnet::math::DEFAULT_NUMERIC bufferMultiplier) {
    if (bufferMultiplier <= 1)
        throw std::invalid_argument("Buffer range multiplier has to be greater than 1");
    using P = std::ranges::range_value_t<R>;
    using SweepLineType = typename __impl::PolygonNeighbours<P>;
    SweepLineType sweepLine;
    std::vector<std::pair<P,P>> out;
    std::vector<BoundingBoxPolygon<P>> boundingBoxPolygons;
    boundingBoxPolygons.reserve(util::size(polygons));
    std::ranges::for_each(polygons,[&boundingBoxPolygons,bufferMultiplier](const auto & p){
        auto aaBBRectangle = Rectangle<fishnet::math::DEFAULT_NUMERIC>(p.aaBB().getPoints());
        boundingBoxPolygons.emplace_back(BoundingBoxPolygon<P>(p,aaBBRectangle.scale(bufferMultiplier)));
    });
    std::ranges::for_each(boundingBoxPolygons,[&sweepLine](const auto & bbPptr){
        sweepLine.addEvent(std::make_unique<__impl::PolygonNeighboursInsertEvent<P>>(bbPptr));
        sweepLine.addEvent(std::make_unique<__impl::PolygonNeighboursRemoveEvent<P>>(bbPptr));
    });
    return sweepLine.sweep(out);
}

}
