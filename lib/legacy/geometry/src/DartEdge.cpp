//
// Created by grube on 14.01.2022.
//

#include "DartEdge.h"

bool DartEdge::operator==(const DartEdge &other) const {
    return this->d1.id == other.d1.id and this->d2.id == other.d2.id or this->d1.id == other.d2.id and this->d2.id == other.d1.id;
}

double DartEdge::length() const {
    return d1.from.position.distance(d1.to.position);
}
