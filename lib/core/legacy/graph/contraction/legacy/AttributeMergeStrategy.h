//
// Created by grube on 14.01.2022.
//

#ifndef WSF_NETWORK_ATTRIBUTEMERGESTRATEGY_H
#define WSF_NETWORK_ATTRIBUTEMERGESTRATEGY_H
#include "graph/vertex/SettlementProperties/SettlementAttributes.h"
/**
 * Interface for the Combination of Settlement Attributes
 */
class AttributeMergeStrategy {
public:
    virtual std::unique_ptr<SettlementAttributes> combine(const SettlementAttributes &a1, const SettlementAttributes &a2) = 0;
};


#endif //WSF_NETWORK_ATTRIBUTEMERGESTRATEGY_H
