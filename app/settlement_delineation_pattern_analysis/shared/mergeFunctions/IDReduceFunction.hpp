#pragma once
#include <fishnet/MultiPolygon.hpp>
#include <fishnet/Polygon.hpp>
#include "SettlementPolygon.hpp"

struct IDReduceFunction{
    template<fishnet::geometry::IPolygon P>
    auto operator()(fishnet::util::forward_range_of<SettlementPolygon<P>> auto && settlementPolygons) const noexcept {
        auto id = std::ranges::begin(settlementPolygons)->key();
        fishnet::geometry::MultiPolygon<P> resultGeometry {settlementPolygons};
        // SettlementPolygon(id,FileReference(-1))
        //TODO...

    }
};