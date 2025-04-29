//
// Created by grube on 15.12.2021.
//

#ifndef BACHELORARBEIT_MERGESTRATEGY_H
#define BACHELORARBEIT_MERGESTRATEGY_H
#include "graph/vertex/Settlement.h"
#include "graph/contraction/AttributeMergeStrategies/AttributeMergeStrategy.h"
#include "graph/contraction/PolygonMergeStrategies/PolygonMergeStrategy.h"
#include <memory>

/**
 * Wrapper for Merging Settlements
 * How the attributes and polygons shall be combined can be specified by providing the desired strategy indidually
 */
class MergeStrategy {
private:
    std::unique_ptr<AttributeMergeStrategy> attributeMergeStrategy; //How attributes should be merged
    std::unique_ptr<PolygonMergeStrategy> polygonMergeStrategy;    //How polygons should be merged
public:
    MergeStrategy(std::unique_ptr<AttributeMergeStrategy> &attributeMerge,
                  std::unique_ptr<PolygonMergeStrategy> &polygonMerge):attributeMergeStrategy(std::move(attributeMerge)), polygonMergeStrategy(std::move(polygonMerge)) {};

    std::shared_ptr<Settlement> merge(const Settlement& s1, const Settlement& s2);
};


#endif //BACHELORARBEIT_MERGESTRATEGY_H
