#pragma once
#include <vector>
#include <fishnet/FunctionalConcepts.hpp>
#include "TaskConfig.hpp"




template<typename GeometryObject>
struct ContractionConfig:public MemgraphTaskConfig{
    constexpr static const char * CONTRACTION_PREDICATES_KEY = "contraction-predicates";
    constexpr static const char * REDUCE_FUNCTION_KEY = "reduce-function";

    std::vector<fishnet::util::BiPredicate_t<GeometryObject>> contractBiPredicates;
    fishnet::util::ReduceFunction_t<std::vector<GeometryObject>> reduceFunction;
    
    ContractionConfig(const json & config):MemgraphTaskConfig(config){

    }
};