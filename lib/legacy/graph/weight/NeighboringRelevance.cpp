//
// Created by grube on 04.01.2022.
//

#include "NeighboringRelevance.h"
#include "DistanceWeight.h"

double NeighboringRelevance::accept(Edge &edge) {
    DistanceWeight distanceWeight = DistanceWeight();
    auto to = edge.getTo();
    return to->value() * 1/distanceWeight.accept(edge);
}

constexpr const char *NeighboringRelevance::fieldName() {
    return "NeighborRel";
}

constexpr OGRFieldType NeighboringRelevance::fieldType() {
    return OFTReal;
}


