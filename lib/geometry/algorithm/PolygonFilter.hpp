#pragma once
#include "SweepLine.hpp"
#include "BoundingBoxPolygon.hpp"
namespace fishnet::geometry {

namespace __impl{

template<IPolygon P>
using PolygonFilter = SweepLine<BoundingBoxPolygon<P>,P,HorizontalAABBOrdering<P>>;

template<IPolygon P,util::BiPredicate<P> BinaryFilter, util::Predicate<P> Filter>
class PolygonFilterInsertEvent:public PolygonFilter<P>::InsertEvent{
private:
    Filter & filter; 
    BinaryFilter & binaryFilter;

public:
    PolygonFilterInsertEvent(const BoundingBoxPolygon<P> & box,BinaryFilter  & binaryCondition, Filter  & condition):PolygonFilter<P>::InsertEvent(box),filter(condition),binaryFilter(binaryCondition){}
    
    virtual void process(PolygonFilter<P> & sweepLine, std::vector<P> & output)const{
        auto polygonUnderTest = this->obj->getPolygon();
        if(not filter(polygonUnderTest))
            return;
        sweepLine.addSLS(this->obj);
        const auto & sls = sweepLine.getSLS();
        for(auto it = sls.lower_bound(this->obj); it != sls.begin() && it != sls.end() /* && this->obj->getBoundingBox().left() <= (*it)->getBoundingBox().right() */; ){
            --it;
            if( not binaryFilter((*it)->getPolygon(),polygonUnderTest))
                return;
        }
        for(auto it = sls.upper_bound(this->obj);it != sls.end() /* && this->obj->getBoundingBox().right() >= (*it)->getBoundingBox().left() */; ++it){
            if(not binaryFilter((*it)->getPolygon(),polygonUnderTest))
                return;
        }
        output.push_back(polygonUnderTest);
    }
    virtual fishnet::math::DEFAULT_NUMERIC eventPoint() const noexcept {
        return this->obj->getBoundingBox().top();
    }
};

template<IPolygon P>
struct PolygonFilterRemoveEvent: public PolygonFilter<P>::DefaultRemoveEvent {
    PolygonFilterRemoveEvent(const BoundingBoxPolygon<P> & box):PolygonFilter<P>::DefaultRemoveEvent(box){}
    virtual fishnet::math::DEFAULT_NUMERIC eventPoint() const noexcept {
        return this->obj->getBoundingBox().bottom();
    }
};
}

struct ContainedOrInHoleFilter{
    template<IPolygon P>
    bool operator()(const P & p, const P & underTest)const noexcept{
        return p != underTest and not p.contains(underTest) and not p.isInHole(underTest);
    }
};

template<PolygonRange R,util::BiPredicate<std::ranges::range_value_t<R>> BinaryFilter, util::Predicate<std::ranges::range_value_t<R>> Filter = util::TruePredicate>
static std::vector<std::ranges::range_value_t<R>> filter( const R & polygons, BinaryFilter binaryCondition, Filter condition = Filter()) noexcept {
    using P = std::ranges::range_value_t<R>;
    using SweepLine_t = typename __impl::PolygonFilter<P>;
    SweepLine_t sweepLine;
    std::vector<P> out;
    std::vector<BoundingBoxPolygon<P>> boundingBoxPolygons;
    boundingBoxPolygons.reserve(util::size(polygons));

    std::ranges::for_each(polygons,[&boundingBoxPolygons](const auto & p){boundingBoxPolygons.emplace_back(p);});
    std::ranges::for_each(boundingBoxPolygons,[&sweepLine,&binaryCondition,&condition](const auto & bbPptr){
        sweepLine.addEvent(std::make_unique<__impl::PolygonFilterInsertEvent<P,BinaryFilter,Filter>>(bbPptr,binaryCondition,condition));
        sweepLine.addEvent(std::make_unique<__impl::PolygonFilterRemoveEvent<P>>(bbPptr));
    });
    return sweepLine.sweep(out);
}

template<PolygonRange R, util::Predicate<std::ranges::range_value_t<R>> Filter>
static std::vector<std::ranges::range_value_t<R>> filter(const R & polygons, Filter condition) noexcept {
    auto alwaysTrue = util::TrueBiPredicate();
    return filter(polygons,alwaysTrue,condition);
}
}