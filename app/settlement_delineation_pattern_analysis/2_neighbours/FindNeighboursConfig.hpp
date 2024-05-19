#pragma once
#include <nlohmann/json.hpp> //MIT License Copyright (c) 2013-2022 Niels Lohmann
#include <magic_enum.hpp> //Copyright (c) 2019 - 2024 Daniil Goncharov
#include "TaskConfig.hpp"
#include "NeighbourPredicateJsonReader.hpp"

using json = nlohmann::json;

template<typename GeometryType>
struct FindNeighboursConfig: public MemgraphTaskConfig{
    constexpr static const char * MAX_DISTANCE_KEY = "maxDistanceMeters";
    constexpr static const char * NEIGHBOURING_PREDICATES_KEY = "neighbouring-predicates";

    double maxEdgeDistance;
    std::vector<fishnet::util::BiPredicate_t<GeometryType>> neighbouringPredicates;

    FindNeighboursConfig()=default;

    FindNeighboursConfig(const json & configDescription):MemgraphTaskConfig(configDescription){
        jsonDescription.at(MAX_DISTANCE_KEY).get_to(this->maxEdgeDistance);
        this->neighbouringPredicates.push_back(DistanceBiPredicate(this->maxEdgeDistance));
        for(const auto & neighbourPredicateJson : jsonDescription.at(NEIGHBOURING_PREDICATES_KEY)){
            std::string predicateName;
            neighbourPredicateJson.at("name").get_to(predicateName);
            auto neighbourPredicate = magic_enum::enum_cast<NeighbouringPredicateType>(predicateName)
                .and_then([&neighbourPredicateJson](NeighbouringPredicateType type){return fromNeighbouringPredicateType<GeometryType>(type,neighbourPredicateJson);});
            if(not neighbourPredicate) {
                throw std::runtime_error("Could not parse json to Neighbouring BiPredicate:\n"+neighbourPredicateJson.dump()+"\nFilter name might not \""+predicateName+"\" be supported");
            }
            this->neighbouringPredicates.push_back(std::move(neighbourPredicate.value()));
        }
    }
};