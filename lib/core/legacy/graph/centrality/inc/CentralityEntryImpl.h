//
// Created by grube on 09.01.2022.
//

#ifndef BACHELORARBEIT_CENTRALITYENTRYIMPL_H
#define BACHELORARBEIT_CENTRALITYENTRYIMPL_H
#include "CentralityEntry.h"

/**
 * Template class for CentralityEntries extending the abstract class CentralityEntry
 * Specifies the datatype of the value and stores the value
 * @tparam T datatype
 */
template<typename T>
class CentralityEntryImpl:public CentralityEntry {
private:
    T value;
public:
    CentralityEntryImpl(CentralityMeasure & measure, T val):CentralityEntry(measure), value(val){};

    /**
     * Sets value in OGRFeature of Network Entity, however the type of value
     * has to be compatible with the datatype specified the centrality measure used.
     * Example: DegreeCentrality: OFTInteger -> CentralityMeasure values should be of type int
     *
     * @param feature
     */
    void setField(OGRFeature *feature) const override {
        feature->SetField(measure.fieldName(), value);
    }

    T getValue();
};

template<typename T>
T CentralityEntryImpl<T>::getValue() {
    return this->value;
}


#endif //BACHELORARBEIT_CENTRALITYENTRYIMPL_H
