//
// Created by grube on 04.01.2022.
//

#include "WSFSettlementValue.h"

double WSFSettlementValue::accept(const Settlement &settlement) {
    auto& attributes = settlement.getAttributes();
    switch (attributes->getType()) {
        case WSFSettlementType::BASE:
            return attributes->getArea();
        case WSFSettlementType::IMPERVIOUSNESS:
            return attributes->getArea()* attributes->getImperviousness();
        case WSFSettlementType::POPULATION:
            return attributes->getArea() + attributes->getArea() * attributes->getPopulation() / POPULATION_SCALE;
        case WSFSettlementType::FULL:
            return attributes->getArea() * attributes->getImperviousness() +
                   attributes->getArea() * attributes->getPopulation() / POPULATION_SCALE;
    }
    return 0.0;
}

bool WSFSettlementValue::supports(WSFSettlementType type) {
    return true;
}

