//
// Created by grube on 08.01.2022.
//

#ifndef BACHELORARBEIT_CENTRALITYENTRY_H
#define BACHELORARBEIT_CENTRALITYENTRY_H
#include <ogr_core.h>
#include "ogrsf_frmts.h"
#include "graph/centrality/CentralityMeasure.h"

/**
 * Abstract Class for handeling Centrality Measure values
 * Storing reference to the centrality measure entity the value is originating from
 */
class CentralityEntry {
protected:
    CentralityMeasure & measure;
public:
    explicit CentralityEntry(CentralityMeasure & centralityMeasure) : measure(centralityMeasure){};
    /**
     * Set Field in Visualization for current CentralityEntry on the given feature
     * @param feature
     */
    virtual void setField(OGRFeature * feature)const = 0 ;
    virtual ~CentralityEntry() = default;;
};



#endif //BACHELORARBEIT_CENTRALITYENTRY_H
