#pragma once
#include "SweepLine.hpp"
#include "BoundingBoxPolygon.hpp"
namespace fishnet::geometry {

namespace __impl{

/**
 * @brief Type for Polygon Filter Sweepline
 * It stores the polygons as BoundingBoxPolygons in the SLS and sorts them from left to right
 * The output is a vector of polygons of type P, which are passing the filter.
 * @tparam P polygon type
 */
template<IPolygon P>
using PolygonFilter = SweepLine<BoundingBoxPolygon<P>,std::vector<P>,HorizontalAABBOrdering<P>>;

/**
 * @brief Insert event for Polygon Filter Sweepline
 * 
 * @tparam P polygon type
 * @tparam BinaryFilter (P,P) -> bool
 * @tparam Filter: (P) -> bool
 */
template<IPolygon P,util::BiPredicate<P> BinaryFilter, util::Predicate<P> Filter>
class PolygonFilterInsertEvent:public PolygonFilter<P>::InsertEvent{
private:
    Filter & filter; 
    BinaryFilter & binaryFilter;

public:
    PolygonFilterInsertEvent(const BoundingBoxPolygon<P> & box,BinaryFilter  & binaryCondition, Filter  & condition):PolygonFilter<P>::InsertEvent(box),filter(condition),binaryFilter(binaryCondition){}
    
    /**
     * @brief processing of this event
     * 
     * @param sweepLine 
     * @param output 
     */
    virtual void process(PolygonFilter<P> & sweepLine, std::vector<P> & output)const{
        const auto & polygonUnderTest = this->obj->getPolygon();
        if(not filter(polygonUnderTest))
            return; // directly return if polygon does not pass filter
        sweepLine.addSLS(this->obj);
        const auto & sls = sweepLine.getSLS();
        for(auto it = sls.lower_bound(this->obj); it != sls.begin() && it != sls.end() /* && this->obj->getBoundingBox().left() <= (*it)->getBoundingBox().right() */; ){
            --it; // skip same element, by first decrementing
            if( not binaryFilter((*it)->getPolygon(),polygonUnderTest))
                return;
        }
        for(auto it = sls.upper_bound(this->obj);it != sls.end() /* && this->obj->getBoundingBox().right() >= (*it)->getBoundingBox().left() */; ++it){
            if(not binaryFilter((*it)->getPolygon(),polygonUnderTest))
                return;
        }
        output.push_back(polygonUnderTest); // add to output if all filters were passed
    }

    /**
     * @brief EventPoint of Insert is the top of the bounding box (Sweepline goes from top to bottom)
     * 
     * @return fishnet::math::DEFAULT_NUMERIC 
     */
    virtual fishnet::math::DEFAULT_NUMERIC eventPoint() const noexcept {
        return this->obj->getBoundingBox().top();
    }
};

template<IPolygon P>
struct PolygonFilterRemoveEvent: public PolygonFilter<P>::DefaultRemoveEvent {
    PolygonFilterRemoveEvent(const BoundingBoxPolygon<P> & box):PolygonFilter<P>::DefaultRemoveEvent(box){}
    /**
     * @brief EventPoint of Removal is the bottom of the bounding box (Sweepline goes from top to bottom)
     * 
     * @return fishnet::math::DEFAULT_NUMERIC 
     */
    virtual fishnet::math::DEFAULT_NUMERIC eventPoint() const noexcept {
        return this->obj->getBoundingBox().bottom();
    }
};
}

/**
 * @brief Filters a range of polygons and returns a list of all polygons that pass the filter(s)
 * 
 * @tparam R range type
 * @tparam BinaryFilter BiPredicate type
 * @tparam Filter Predicate type
 * @param polygons range of polygons of type P
 * @param binaryCondition 
 * @param condition 
 * @return std::vector<P>
 */
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

/**
 * @brief Polygon Filter overload with only a unary filter
 * 
 * @tparam R range type
 * @tparam Filter Predicate type
 * @param polygons range of Polygons of type P
 * @param condition 
 * @return std::vector<P>
 */
template<PolygonRange R, util::Predicate<std::ranges::range_value_t<R>> Filter>
static std::vector<std::ranges::range_value_t<R>> filter(const R & polygons, Filter condition) noexcept {
    auto alwaysTrue = util::TrueBiPredicate();
    return filter(polygons,alwaysTrue,condition);
}
}