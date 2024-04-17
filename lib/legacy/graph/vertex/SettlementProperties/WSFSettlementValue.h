//
// Created by grube on 04.01.2022.
//

#ifndef BACHELORARBEIT_WSFSETTLEMENTVALUE_H
#define BACHELORARBEIT_WSFSETTLEMENTVALUE_H
#include "SettlementValue.h"
#include "graph/vertex/Settlement.h"
/**
 * Define SettlementValue as a combination of the settlement's attributes
 */
class WSFSettlementValue : public SettlementValue {
private:
    double POPULATION_SCALE = 500.0; //reduced impact of population values



public:
    double accept(const Settlement &settlement) override;
    bool supports(WSFSettlementType type) override;

};


#endif //BACHELORARBEIT_WSFSETTLEMENTVALUE_H
