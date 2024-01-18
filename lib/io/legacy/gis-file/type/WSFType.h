//
// Created by grube on 23.09.2021.
//

#ifndef BACHELORARBEIT_WSFTYPE_H
#define BACHELORARBEIT_WSFTYPE_H
#include "WSFTypeEnum.h"
#include "gdal.h"

/**
 * Wrapper for WSFTypeEnum to allow for Enum functions (not possible for C++ enums)
 */
class WSFType{
private:
    WSFTypeEnum wsfInputTypeEnum;
public:
    explicit WSFType(WSFTypeEnum type);

    WSFType();
    std::string string();
    OGRFieldType datatype();
    WSFTypeEnum type();
};



#endif //BACHELORARBEIT_WSFTYPE_H
