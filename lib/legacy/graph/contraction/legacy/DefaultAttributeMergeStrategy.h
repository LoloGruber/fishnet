//
// Created by grube on 14.01.2022.
//

#ifndef WSF_NETWORK_DEFAULTATTRIBUTEMERGESTRATEGY_H
#define WSF_NETWORK_DEFAULTATTRIBUTEMERGESTRATEGY_H
#include "AttributeMergeStrategy.h"
/**
 * Default Attribute Combination Strategy yielding:
 * AREA = a1.area + a2.area
 * IMP = AVG Imperviousness of a1 and a2 weighted with the area of a1 and a2
 * POP = a1.pop + a2.pop
 */
class DefaultAttributeMergeStrategy: public AttributeMergeStrategy {
public:
    std::unique_ptr<SettlementAttributes>
    combine(const SettlementAttributes &a1, const SettlementAttributes &a2) override;

};


#endif //WSF_NETWORK_DEFAULTATTRIBUTEMERGESTRATEGY_H
