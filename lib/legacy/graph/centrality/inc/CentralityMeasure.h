//
// Created by grube on 07.01.2022.
//

#ifndef BACHELORARBEIT_CENTRALITYMEASURE_H
#define BACHELORARBEIT_CENTRALITYMEASURE_H
#include <ogr_core.h>
#include "ogrsf_frmts.h"
class Network;
/**
 * Abstract Class defining Centrality Measure Computation on a network
 */
class CentralityMeasure {
public:
    /**
     * Create Field in OGRLayer with fieldName() and fieldType() -> which are implemented by derived classes
     * @param layer
     */
    void createField(OGRLayer *layer);

    /**
     * Computes Centrality Measure values for the network
     * The values have to be stored within the Network Entity (i.e. Settlement or Edge or both) as CentralityEntry
     * @param network
     */
    virtual void compute(std::shared_ptr<Network> network)= 0;

    /**
     * Derived classes have to specify the name of the field in the ogrfeature
     * @return
     */
    virtual constexpr const char *fieldName() = 0;

    /**
     * Derived class have to specify the GDALDatatype of the Compute Centrality Measures
     * @return
     */
    virtual constexpr OGRFieldType fieldType() = 0;

    virtual ~CentralityMeasure() = default;
};



#endif //BACHELORARBEIT_CENTRALITYMEASURE_H
