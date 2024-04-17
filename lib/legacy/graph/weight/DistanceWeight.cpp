//
// Created by grube on 04.01.2022.
//

#include "DistanceWeight.h"

double DistanceWeight::accept(Edge & edge) {
    return edge.getFrom()->distance(*edge.getTo());
}

constexpr const char *DistanceWeight::fieldName() {
    return "Distance";
}

constexpr OGRFieldType DistanceWeight::fieldType() {
    return OFTReal;
}

