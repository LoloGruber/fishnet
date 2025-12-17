//
// Created by grube on 04.01.2022.
//

#ifndef BACHELORARBEIT_SETTLEMENTVALUE_H
#define BACHELORARBEIT_SETTLEMENTVALUE_H
class Settlement;
#include "graph/vertex/Settlement.h"
/**
 * Interface to define the computation of Settlement values
 */
class SettlementValue {
public:
    /**
     *
     * @param settlement
     * @return double value calculated on the attributes of the settlement
     */
    virtual double accept(const Settlement & settlement )=0;

    /**
     *
     * @param type
     * @return whether the type of settlement is supported
     */
    virtual bool supports(WSFSettlementType type) = 0;
};



#endif //BACHELORARBEIT_SETTLEMENTVALUE_H
