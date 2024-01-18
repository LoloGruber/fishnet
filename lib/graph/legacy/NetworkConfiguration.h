//
// Created by grube on 25.11.2021.
//

#ifndef BACHELORARBEIT_NETWORKCONFIGURATION_H
#define BACHELORARBEIT_NETWORKCONFIGURATION_H
#include "graph/contraction/MergeStrategy.h"
#include "graph/contraction/PolygonMergeStrategies/TwoShortestLinesMergeStrategy.h"
#include "graph/edge/Weight/DistanceWeight.h"
#include "graph/contraction/PolygonMergeStrategies/CharacteristicPolygonMergeStrategy.h"
#include "graph/contraction/AttributeMergeStrategies/DefaultAttributeMergeStrategy.h"

/**
 * Responsible for storing the configuration of the network specified through parameters
 */
struct NetworkConfiguration {
    double MERGE_DISTANCE = 50.0; //maximum Distance to merge in meters
    int EDGES_PER_NODE = 3; //maximum edges per node
    double CONNECT_DISTANCE = 500.0; //maximum Distance to connect settlements in meters
    std::unique_ptr<MergeStrategy> mergeStrategy;
    std::shared_ptr<SettlementValue> settlementValueStrategy;
    std::shared_ptr<Weight> weightStrategy;

    NetworkConfiguration(int maxEdges, double distanceToConenct, double distanceToMerge, std::unique_ptr<MergeStrategy> & merge, std::unique_ptr<SettlementValue> & settlementValueStrategy, std::shared_ptr<Weight> weightStrat)
    : EDGES_PER_NODE(maxEdges), CONNECT_DISTANCE(distanceToConenct), MERGE_DISTANCE(distanceToMerge), mergeStrategy(std::move(merge)), settlementValueStrategy(std::move(settlementValueStrategy)), weightStrategy(std::move(weightStrat)) {};

    NetworkConfiguration() {
        settlementValueStrategy = std::make_shared<WSFSettlementValue>();;
        std::unique_ptr<PolygonMergeStrategy> polygonMerge = std::make_unique<CharacteristicPolygonMergeStrategy>();
        std::unique_ptr<AttributeMergeStrategy> attributeMerge = std::make_unique<DefaultAttributeMergeStrategy>();
        mergeStrategy = std::move(std::make_unique<MergeStrategy>(attributeMerge,polygonMerge));
        weightStrategy = std::move(std::make_unique<DistanceWeight>());
    }
};

#endif //BACHELORARBEIT_NETWORKCONFIGURATION_H
