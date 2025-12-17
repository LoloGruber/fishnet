//
// Created by grube on 14.01.2022.
//

#include "DefaultAttributeMergeStrategy.h"

std::unique_ptr<SettlementAttributes>
DefaultAttributeMergeStrategy::combine(const SettlementAttributes &a1, const SettlementAttributes &a2) {
    double avgImp = -1;
    int combinedPop = 0;
    double combinedArea = a1.getArea() + a2.getArea();
    if (SettlementAttributes::isValidImperviousness(a1.getImperviousness()) and SettlementAttributes::isValidImperviousness(a2.getImperviousness())) {
        /* weighted average imperviousness if both settlements have imperviousness values stored*/
        avgImp = a1.getImperviousness() * (a1.getArea() / combinedArea) +
                 a2.getImperviousness() * (a2.getArea() / combinedArea);
    } else if (SettlementAttributes::isValidImperviousness(a1.getImperviousness())) {
        avgImp = a1.getImperviousness();
    } else if (SettlementAttributes::isValidImperviousness(a2.getImperviousness())) {
        avgImp = a2.getImperviousness();
    }
    combinedPop = a1.getPopulation() + a2.getPopulation();
    return std::make_unique<SettlementAttributes>(combinedArea, avgImp, combinedPop);
}
