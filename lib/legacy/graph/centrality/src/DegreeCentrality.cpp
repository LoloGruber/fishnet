//
// Created by grube on 07.01.2022.
//

#include "DegreeCentrality.h"
#include "graph/Network.h"
#include "graph/centrality/CentralityEntry/CentralityEntryImpl.h"


void DegreeCentrality::compute(std::shared_ptr<Network> network) {
    std::map<const std::shared_ptr<Settlement> , int> values;
    /* Initially set the degree centrality for each settlement to 0*/
    for (auto &settlement: network->getSettlements()) {
        values.insert(std::make_pair(settlement, 0));
    }

    /* Iterate over all edges and increase the degree centrality of the involved settlements*/
    for (auto &edge: network->getEdges()) {
        if (edge->isDirected()) {
            /* Directed edges increase only the degree centrality of the endpoint, counting the incoming edges (IN-Degree)*/
            auto it = values.find(edge->getTo());
            it->second = it->second + 1;
        } else {
            /* Undirected edges: Degree centrality for both settlements incremented*/
            auto from = values.find(edge->getFrom());
            auto to = values.find(edge->getTo());
            from->second = from->second + 1;
            to->second = to->second + 1;
        }
    }
    for (auto &pair: values) {
        /* Store one CentralityEntry in each Settlement with type <int> and reference to the used centrality measure <*this>*/
        pair.first->consume(std::make_unique<CentralityEntryImpl<int>>(*this, pair.second));
    }
}

constexpr const char *DegreeCentrality::fieldName() {
    return "DegreeCent";
}

constexpr OGRFieldType DegreeCentrality::fieldType() {
    return OFTInteger;
}
