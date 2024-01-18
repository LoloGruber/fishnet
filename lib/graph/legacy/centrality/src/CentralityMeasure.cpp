//
// Created by grube on 08.01.2022.
//

#include "CentralityMeasure.h"

void CentralityMeasure::createField(OGRLayer *layer) {
    auto def = OGRFieldDefn(fieldName(), fieldType());
    layer->CreateField(&def);
}
