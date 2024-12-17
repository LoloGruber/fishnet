#pragma once
#include <optional>
#include <nlohmann/json.hpp>
#include <fishnet/FunctionalConcepts.hpp>
#include "NeighbourPredicateType.hpp"
#include "DistanceBiPredicate.hpp"

template<typename T>
static std::optional<fishnet::util::BiPredicate_t<T>> fromNeighbouringPredicateType(NeighbouringPredicateType type,const nlohmann::json & predicateDesc, const DistanceFunction & distanceFunction = MetricDistance()) {
    switch(type){
        case NeighbouringPredicateType::DistanceBiPredicate:
            double maxDistance;
            predicateDesc.at("distance").get_to(maxDistance);
            return DistanceBiPredicate(distanceFunction,maxDistance);
    }
    return std::nullopt;
}
