//
// Created by grube on 14.01.2022.
//

#ifndef WSF_NETWORK_CHARACTERISTICPOLYGONMERGESTRATEGY_H
#define WSF_NETWORK_CHARACTERISTICPOLYGONMERGESTRATEGY_H
#include "PolygonMergeStrategy.h"
/**
 * Implementation of PolygonMergeStrategy
 * Return the characteristic shape on the combined set of points
 */
class CharacteristicPolygonMergeStrategy : public PolygonMergeStrategy {
public:
    std::unique_ptr<Polygon> merge(const Polygon &u, const Polygon &v) override;
};


#endif //WSF_NETWORK_CHARACTERISTICPOLYGONMERGESTRATEGY_H
