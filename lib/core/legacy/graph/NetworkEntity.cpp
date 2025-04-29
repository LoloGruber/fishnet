//
// Created by grube on 08.01.2022.
//

#include "graph/NetworkEntity.h"

void NetworkEntity::consume(std::unique_ptr<CentralityEntry> centralityValue) {
    this->centralityValues.push_back(std::move(centralityValue));
}

const std::vector<std::unique_ptr<CentralityEntry>> & NetworkEntity::getCentralityValues() {
    return this->centralityValues;
}