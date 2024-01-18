//
// Created by grube on 06.01.2022.
//
#include "Weight.h"


void Weight::setField(OGRLayer * layer, OGRFeature *feature, Edge &edge) {
    /* If field was not created in advance -> create Field now*/
    if (feature->GetFieldIndex(fieldName()) == -1) {
        createField(layer);
    }
    feature->SetField(EntityField::entityFieldName, "edge");
    feature->SetField(fieldName(), accept(edge));
}

void Weight::createField(OGRLayer *layer) {
    auto def = OGRFieldDefn(fieldName(), fieldType());
    layer->CreateField(&def);
}
