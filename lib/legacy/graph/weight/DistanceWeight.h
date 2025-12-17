//
// Created by grube on 04.01.2022.
//

#ifndef BACHELORARBEIT_DISTANCEWEIGHT_H
#define BACHELORARBEIT_DISTANCEWEIGHT_H
#include "Weight.h"
/**
 * Weight edge according to the distance it covers
 */
class DistanceWeight : public Weight {
public:
    double accept(Edge & edge) override;

    constexpr const char *fieldName() override;

    constexpr OGRFieldType fieldType() override;
};


#endif //BACHELORARBEIT_DISTANCEWEIGHT_H
