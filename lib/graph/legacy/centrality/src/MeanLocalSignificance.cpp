//
// Created by grube on 10.01.2022.
//

#include <unordered_map>
#include "MeanLocalSignificance.h"
#include "graph/Network.h"
#include "graph/centrality/CentralityEntry/CentralityEntryImpl.h"

void MeanLocalSignificance::compute(std::shared_ptr<Network> network) {
    std::unordered_map<std::shared_ptr<Settlement>, std::pair<double, int>> values;
    /*each entry consists out of one settlement as key and the values: <Sum of Local Significance, Amount of Local Significance values> */

    /* Initialize mean local significance to 0*/
    for (auto &settlement: network->getSettlements()) {
        values.insert(std::make_pair(settlement, std::make_pair(0, 0)));
    }
    for (auto &edge: network->getEdges()) {
        auto itFrom = values.find(edge->getFrom());
        auto itTo = values.find(edge->getTo());

        //LocalSignificance of Settlements u and v = u.Area() * v.Area() / u.distance(v)^2
        double localSignificance =(itFrom->first->getAttributes()->getArea() * itTo->first->getAttributes()->getArea() /
                                   pow(itFrom->first->distance(*itTo->first),2));
        if (edge->isDirected()) {
            /* Directed edges increase only the mean local significance of the endpoint, counting the incoming edges */
            itTo->second.first = itTo->second.first + localSignificance; // add localSignificance to sum
            itTo->second.second += 1; // increase the amount of localSignificance values
        } else {
            itTo->second.first += localSignificance;
            itTo->second.second += 1;
            itFrom->second.first += localSignificance;
            itFrom->second.second += 1;
        }
    }
    for (auto &entry: values) {
        if (entry.second.second == 0) {
            /* If no values are added for the current settlement -> store 0.0 as CentralityEntry with reference to this Class as the origin of that value*/
            entry.first->consume(std::make_unique<CentralityEntryImpl<double>>(*this,0.0));
        } else {
            entry.first->consume(
                    std::make_unique<CentralityEntryImpl<double>>(*this,log10( entry.second.first / entry.second.second))); //log10 applied to bound larger numbers
        }
    }
}

constexpr const char *MeanLocalSignificance::fieldName() {
    return "LocalSig";
}

constexpr OGRFieldType MeanLocalSignificance::fieldType() {
    return OFTReal;
}
