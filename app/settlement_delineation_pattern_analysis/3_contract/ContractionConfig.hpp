#pragma once
#include <vector>
#include <fishnet/FunctionalConcepts.hpp>
#include "TaskConfig.hpp"




template<typename InputGeometryType>
struct ContractionConfig:public MemgraphTaskConfig{
    constexpr static const char * CONTRACTION_PREDICATES_KEY = "contraction-predicates";
    constexpr static const char * WORKERS_KEY = "workers";

    std::vector<fishnet::util::BiPredicate_t<InputGeometryType>> contractBiPredicates;
    u_int8_t workers;
    
    ContractionConfig(const json & config):MemgraphTaskConfig(config){

    }
};