//
// Created by grube on 10.01.2022.
//

#ifndef BACHELORARBEIT_MEANLOCALSIGNIFICANCE_H
#define BACHELORARBEIT_MEANLOCALSIGNIFICANCE_H
#include "graph/centrality/CentralityMeasure.h"
/**
 * See CentralityMeasure for the documentation of the implemented functions
 */
class MeanLocalSignificance:public CentralityMeasure {
    void compute(std::shared_ptr<Network> network) override;

    constexpr const char *fieldName() override;

    constexpr OGRFieldType fieldType() override;
};


#endif //BACHELORARBEIT_MEANLOCALSIGNIFICANCE_H
