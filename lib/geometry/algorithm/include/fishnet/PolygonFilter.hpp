#pragma once
#include "SweepLine.hpp"
#include "BoundingBoxWrapper.hpp"
namespace fishnet::geometry {

namespace __impl{


// template<Shape S>
// using ShapeFilter = SweepLine<BoundingBoxWrapper<S>,std::vector<S>,HorizontalAABBOrdering<BoundingBoxWrapper<S>>>;

/**
 * @brief Type for Geometry Filter Sweepline
 * It stores the shapes as BoundingBoxWrapper objects in the SLS and sorts them from left to right
 * The output is a vector of the element type of the bounding box wrapper, which are passing the filter.
 * @tparam BoundingBoxWrapper type
 */
template<IBoundingBoxWrapper BBWrapper>
using GeometryFilter = SweepLine<BBWrapper, std::vector<typename BBWrapper::element_type>,HorizontalAABBOrdering<BBWrapper>>;


/**
 * @brief Insert event for Geometry Filter Sweepline
 * 
 * @tparam P polygon type
 * @tparam BinaryFilter (P,P) -> bool
 * @tparam Filter: (P) -> bool
 */
template<IBoundingBoxWrapper BBWrapper,util::BiPredicate<typename BBWrapper::geometry_type> BinaryFilter, util::Predicate<typename BBWrapper::geometry_type> Filter>
class ShapeFilterInsertEvent:public GeometryFilter<BBWrapper>::InsertEvent{
private:
    Filter & filter; 
    BinaryFilter & binaryFilter;

public:
    ShapeFilterInsertEvent(const BBWrapper & box,BinaryFilter  & binaryCondition, Filter  & condition):GeometryFilter<BBWrapper>::InsertEvent(box),filter(condition),binaryFilter(binaryCondition){}
    
    /**
     * @brief processing of this event
     * 
     * @param sweepLine 
     * @param output 
     */
    virtual void process(GeometryFilter<BBWrapper> & sweepLine, std::vector<typename BBWrapper::element_type> & output)const{
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
        output.push_back(this->obj->getElement()); // add to output if all filters were passed
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

template<IBoundingBoxWrapper BBWrapper>
struct ShapeFilterRemoveEvent: public GeometryFilter<BBWrapper>::DefaultRemoveEvent {
    ShapeFilterRemoveEvent(const BBWrapper & box):GeometryFilter<BBWrapper>::DefaultRemoveEvent(box){}
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
    using SweepLine_t = typename __impl::GeometryFilter<BoundingBoxWrapper<P>>;
    SweepLine_t sweepLine;
    std::vector<P> out;
    std::vector<BoundingBoxWrapper<P>> boundingBoxPolygons;
    boundingBoxPolygons.reserve(util::size(polygons));

    std::ranges::for_each(polygons,[&boundingBoxPolygons](const auto & p){boundingBoxPolygons.emplace_back(p);});
    std::ranges::for_each(boundingBoxPolygons,[&sweepLine,&binaryCondition,&condition](const auto & bbPptr){
        sweepLine.addEvent(std::make_unique<__impl::ShapeFilterInsertEvent<BoundingBoxWrapper<P>,BinaryFilter,Filter>>(bbPptr,binaryCondition,condition));
        sweepLine.addEvent(std::make_unique<__impl::ShapeFilterRemoveEvent<BoundingBoxWrapper<P>>>(bbPptr));
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


template<std::ranges::forward_range R, class Proj, geometry::Shape G = std::invoke_result_t<Proj,std::ranges::range_value_t<std::remove_cvref_t<R>>>>
static std::vector<std::ranges::range_value_t<R>> filter(const R & elements, const Proj & proj, util::BiPredicate<G,G> auto binaryCondition, util::Predicate<G> auto condition) {
    using T = std::ranges::range_value_t<R>;
    using SweepLine_t = __impl::GeometryFilter<BoundingBoxWrapper<T,Proj>>;
    using BinaryFilter = decltype(binaryCondition);
    using Filter = decltype(condition);
    SweepLine_t sweepLine;
    std::vector<T> out;
    std::vector<BoundingBoxWrapper<T,Proj>> boundingBoxWrappers;
    boundingBoxWrappers.reserve(util::size(elements));
    std::ranges::for_each(elements,[&boundingBoxWrappers](const auto & p){boundingBoxWrappers.emplace_back(p);});
    std::ranges::for_each(boundingBoxWrappers,[&sweepLine,&binaryCondition,&condition](const auto & bbPptr){
        sweepLine.addEvent(std::make_unique<__impl::ShapeFilterInsertEvent<BoundingBoxWrapper<T,Proj>,BinaryFilter,Filter>>(bbPptr,binaryCondition,condition));
        sweepLine.addEvent(std::make_unique<__impl::ShapeFilterRemoveEvent<BoundingBoxWrapper<T,Proj>>>(bbPptr));
    });
    return sweepLine.sweep(out);
}  
}