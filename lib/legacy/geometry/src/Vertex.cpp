//
// Created by grube on 15.01.2022.
//

#include "Vertex.h"

bool Vertex::operator==(const Vertex &other) const {
    return this->id == other.id;
}
