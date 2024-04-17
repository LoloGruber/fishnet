//
// Created by grube on 04.01.2022.
//

#ifndef BACHELORARBEIT_NEIGHBORINGRELEVANCE_H
#define BACHELORARBEIT_NEIGHBORINGRELEVANCE_H
#include "Weight.h"
/**
 * Weight edge according to the value of the settlement it points to, divided by the distance
 * -> High values for edges that connect to influential and nearby settlements
 */
class NeighboringRelevance : public  Weight {
public:
    double accept(Edge &edge) override;

    constexpr const char *fieldName() override;

    constexpr OGRFieldType fieldType() override;
};


#endif //BACHELORARBEIT_NEIGHBORINGRELEVANCE_H
