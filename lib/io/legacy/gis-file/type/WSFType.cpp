//
// Created by grube on 23.09.2021.
//
#include "WSFType.h"

std::string WSFType::string() {
    switch (wsfInputTypeEnum) {
        case WSFTypeEnum::AREA:
            return "wsf_area";
            case WSFTypeEnum::POPULATION:
                return "wsf_pop";
                case WSFTypeEnum::IMPERVIOUSNESS:
                    return "wsf_imp";
        case WSFTypeEnum::NETWORK:
            return "wsf_network";
        case WSFTypeEnum::OTHER:
            return "other";
    }
    return "";
}

OGRFieldType WSFType::datatype() {
    //    switch (wsfInputTypeEnum) {
    //        case WSFInputTypeEnum::SETTLEMENT:
    //            return OFTInteger;
    //        case WSFInputTypeEnum::POPULATION:
    //            return OFTInteger;
    //        case WSFInputTypeEnum::IMPERVIOUSNESS:
    //            return OFTInteger;
    //    }
    return OFTInteger;
}

WSFType::WSFType(WSFTypeEnum type) {
    this->wsfInputTypeEnum = type;
}

WSFTypeEnum WSFType::type() {
    return this->wsfInputTypeEnum;
}

WSFType::WSFType() {
    this->wsfInputTypeEnum = WSFTypeEnum::OTHER;

}
