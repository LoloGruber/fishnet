#pragma once
#include <fishnet/MultiPolygon.hpp>
#include <fishnet/Polygon.hpp>
#include "SettlementPolygon.hpp"

struct IDReduceFunction{
    FileReference outputFileRef;

    IDReduceFunction(FileReference outputFileRef):outputFileRef(std::move(outputFileRef)){}

    template<fishnet::geometry::IPolygon P>
    SettlementPolygon<fishnet::geometry::MultiPolygon<P>> operator()( const std::vector<SettlementPolygon<P>> & settlementPolygons) const noexcept {
        size_t id = std::ranges::fold_left(settlementPolygons,0,[](size_t current, const auto & settlementPolygon){return current + settlementPolygon.key();}) + 1;
        fishnet::geometry::MultiPolygon<P> resultGeometry {settlementPolygons | std::views::transform([](const auto & settPoly){return static_cast<P>(settPoly);})};
        return SettlementPolygon<fishnet::geometry::MultiPolygon<P>>(id,outputFileRef,resultGeometry);
    }
};

/*
with _From = IDReduceFunction; _To = 
std::function<SettlementPolygon<fishnet::geometry::MultiPolygon<fishnet::geometry::Polygon<double> > >(const std::vector<SettlementPolygon<fishnet::geometry::Polygon<double> >, std::allocator<SettlementPolygon<fishnet::geometry::Polygon<double> > > >&)>

*/
