//
// Created by grube on 14.01.2022.
//

#include "Dart.h"

double Dart::angle() const{
    return to.position.angle(from.position);
}

bool Dart::operator<(const Dart &other) const {
    return this->angle() < other.angle();
}

bool Dart::operator==(const Dart &other)const {
    return this->id == other.id;
}
