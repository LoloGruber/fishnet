//
// Created by grube on 07.01.2022.
//
#include "EntityField.h"
const char *EntityField::entityFieldName = static_cast<const char *> ("type");

void EntityField::init(OGRLayer *layer) {
    auto def = OGRFieldDefn(EntityField::entityFieldName, EntityField::entityFieldType);
    layer->CreateField(&def);
}
