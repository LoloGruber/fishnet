//
// Created by grube on 07.01.2022.
//

#ifndef BACHELORARBEIT_ENTITYFIELD_H
#define BACHELORARBEIT_ENTITYFIELD_H

#include <ogr_core.h>
#include <ogrsf_frmts.h>

/**
 * Specifies the field in the visualization that sets the type of a network entity (Settlement or Edge)
 */
struct EntityField{
    static const char *entityFieldName;
    static  const OGRFieldType entityFieldType = OFTString;
    static void init(OGRLayer *layer);
};

#endif //BACHELORARBEIT_ENTITYFIELD_H
