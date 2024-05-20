#pragma once
#include <vector>
#include <nlohmann/json.hpp> //MIT License Copyright (c) 2013-2022 Niels Lohmann
#include <magic_enum.hpp> //Copyright (c) 2019 - 2024 Daniil Goncharov
#include <fishnet/FunctionalConcepts.hpp>
#include "TaskConfig.hpp"
#include "NeighbourPredicateJsonReader.hpp"

/**
 * @brief Contraction configuration parser
 * 
 * @tparam GeometryType 
 */
template<typename GeometryType>
struct ContractionConfig:public MemgraphTaskConfig{
    constexpr static const char * CONTRACTION_PREDICATES_KEY = "contraction-predicates";
    constexpr static const char * WORKERS_KEY = "merge-workers";

    std::vector<fishnet::util::BiPredicate_t<GeometryType>> contractBiPredicates;
    u_int8_t workers;
    
    ContractionConfig(const json & config):MemgraphTaskConfig(config){
        this->jsonDescription.at(WORKERS_KEY).get_to(this->workers);
        for(const auto & contractPredicateJson : jsonDescription.at(CONTRACTION_PREDICATES_KEY)){
            std::string predicateName;
            contractPredicateJson.at("name").get_to(predicateName);
            auto contractionPredicate = magic_enum::enum_cast<NeighbouringPredicateType>(predicateName)
                .and_then([&contractPredicateJson](NeighbouringPredicateType type){return fromNeighbouringPredicateType<GeometryType>(type,contractPredicateJson);});
            if(not contractionPredicate) {
                throw std::runtime_error("Could not parse json to Neighbouring BiPredicate:\n"+contractPredicateJson.dump()+"\nFilter name might not \""+predicateName+"\" be supported");
            }
            this->contractBiPredicates.push_back(std::move(contractionPredicate.value()));
        }
    }
};