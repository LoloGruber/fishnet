//
// Created by grube on 07.01.2022.
//

#ifndef BACHELORARBEIT_DEGREECENTRALITY_H
#define BACHELORARBEIT_DEGREECENTRALITY_H
#include "graph/centrality/CentralityMeasure.h"
/**
 * See CentralityMeasure for the documentation of the implemented functions
 */
class DegreeCentrality: public CentralityMeasure {
public:
    void compute(std::shared_ptr<Network> network) override;

    constexpr const char *fieldName() override;

    constexpr OGRFieldType fieldType() override;

};


#endif //BACHELORARBEIT_DEGREECENTRALITY_H
