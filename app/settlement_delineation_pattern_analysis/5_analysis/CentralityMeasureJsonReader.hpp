#pragma once
#include <optional>
#include "CentralityMeasureType.hpp"
#include "DegreeCentralityMeasure.hpp"
#include "MeanLocalSignificance.hpp"
#include "SmallerNeighboursRatio.hpp"
#include <nlohmann/json.hpp>

template<typename GraphType, typename GeometryType>
static std::optional<CentralityMeasure_t<GraphType,GeometryType>> fromJson(CentralityMeasureType type, const nlohmann::json & desc){
    switch(type){
        case CentralityMeasureType::DegreeCentrality:
            return DegreeCentralityMeasure();
        case CentralityMeasureType::MeanLocalSignificance:
            return MeanLocalSignificance();
        case CentralityMeasureType::SmallerNeighboursRatio:
            return SmallerNeighboursRatio();

    }
    return std::nullopt;
}