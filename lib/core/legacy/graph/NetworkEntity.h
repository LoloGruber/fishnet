//
// Created by grube on 08.01.2022.
//

#ifndef BACHELORARBEIT_NETWORKENTITY_H
#define BACHELORARBEIT_NETWORKENTITY_H


#include <list>
#include "graph/centrality/CentralityEntry/CentralityEntry.h"

/**
 * Base class for all entities that can store centrality values (Settlements and Edges)
 */
class NetworkEntity {
protected:
    std::vector<std::unique_ptr<CentralityEntry>> centralityValues;
public:
    /**
     * Add CentralityValue to the list of values
     * @param centralityValue to be added
     */
    void consume(std::unique_ptr<CentralityEntry> centralityValue);
    const std::vector<std::unique_ptr<CentralityEntry>> &getCentralityValues();
    virtual ~NetworkEntity()=default;
};


#endif //BACHELORARBEIT_NETWORKENTITY_H
