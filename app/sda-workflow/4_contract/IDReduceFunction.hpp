#pragma once
#include <fishnet/MultiPolygon.hpp>
#include <fishnet/Polygon.hpp>
#include "SettlementPolygon.hpp"
#include "Task.hpp"

/**
 * @brief Reduce function Functor that merges a set of settlements polygons into a single settlement multi-polygon.
 * The resulting settlement has the accumulated id of all merged settlements
 * 
 */
struct IDReduceFunction{
    FileReference outputFileRef;
    
    IDReduceFunction(FileReference outputFileRef):outputFileRef(std::move(outputFileRef)){}

    template<fishnet::geometry::IPolygon P>
    SettlementPolygon<fishnet::geometry::MultiPolygon<P>> operator()( const std::vector<SettlementPolygon<P>> & settlementPolygons) const noexcept {
        size_t id = std::ranges::fold_left(settlementPolygons,0,[](size_t current, const auto & settlementPolygon){return current + settlementPolygon.key();}); 
        id = normalizeToShpFileIntField(id);
        fishnet::geometry::MultiPolygon<P> resultGeometry {settlementPolygons | std::views::transform([](const auto & settPoly){return static_cast<P>(settPoly);}),true};
        return SettlementPolygon<fishnet::geometry::MultiPolygon<P>>(id,outputFileRef,resultGeometry);
    }
};