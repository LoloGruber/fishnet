//
// Created by grube on 25.11.2021.
//

#include "Network.h"
#include "graph/edge/Weight/NeighboringRelevance.h"
#include "graph/edge/Weight/DistanceWeight.h"
#include "io/ProgressPrinter.h"
#include <iostream>
#include <set>
#include <queue>

const Network::SettlementSet &Network::getSettlements() const{
    return this->settlements;
}

const Network::EdgeSet & Network::getEdges() const {
    return this->edges;
}
void Network::addSettlement(const std::shared_ptr<Settlement> &settlement) {
    if (not containsSettlement(settlement)) {
        this->settlements.insert(settlement);
    }
}

bool Network::containsSettlement(const std::shared_ptr<Settlement> & settlement) {
    return this->settlements.contains(settlement);
/*    for (auto &s: this->settlements) {
        if (*s == settlement) {
            return true;
        }
    }
    return false;*/
}

void Network::removeSettlement(const std::shared_ptr<Settlement> & settlement) {
    if (containsSettlement(settlement)) {
        /* remove all edges connecting with the settlement to be deleted*/
        for(auto &e : getEdgesOfSettlement(settlement)) {
            removeEdge(e);
        }
        this->settlements.erase(settlement);
    }
}

void Network::addEdge(std::shared_ptr<Settlement> &from,std::shared_ptr<Settlement> &to, bool directed) {
    this->addEdge(std::make_shared<Edge>(from, to, directed));
}

void Network::addEdge(std::shared_ptr<Edge> edge) {
    if (not containsEdge(edge) and containsSettlement(edge->getFrom()) and containsSettlement(edge->getTo())) {
        this->edges.insert(std::move(edge));
    }
}
void Network::addEdges(std::vector<std::shared_ptr<Edge>> &newEdges) {
    for (auto &e: edges) {
        addEdge(e);
    }
}

void Network::removeEdge(const std::shared_ptr<Settlement> &from,const std::shared_ptr<Settlement> &to, bool directed) {
    auto edge = std::make_shared<Edge>(from, to, directed);
    this->removeEdge(edge);
}

void Network::removeEdge(const std::shared_ptr< Edge> &edge) {
    this->edges.erase(edge);

}

bool Network::containsEdge(std::shared_ptr<Settlement> &from, std::shared_ptr<Settlement> &to, bool directed) {
    return containsEdge(std::make_shared<Edge>(from, to, directed));
}

bool Network::containsEdge(const std::shared_ptr< Edge> &edge) {
    return this->edges.contains(edge);
}

std::vector<std::shared_ptr<Edge>> Network::getEdgesOfSettlement(const std::shared_ptr<Settlement> &settlement) {
    std::vector<std::shared_ptr<Edge>> edgesToSettlement;
    for (auto &edge: this->edges) {
        if (edge->contains(settlement)) {
            edgesToSettlement.push_back(edge);
        }
    }
    return edgesToSettlement;

}

void Network::createEdges(bool directed) {
    auto progress = ProgressPrinter(settlements.size(),"Creating edges for " + std::to_string(settlements.size()) + " Settlements");
    /* Use neighboring relevance to examine important neighbors*/
    std::shared_ptr<Weight> neighborRelevance = std::make_shared<NeighboringRelevance>();
    /* Comparator to infer the most important neighbors after sorting*/
    auto cmpInverse = [&neighborRelevance] (const std::shared_ptr<Edge> & e1, const std::shared_ptr<Edge> & e2) {
        return e1->weigth(neighborRelevance) > e2->weigth(neighborRelevance);
    };
    for (auto &current: this->settlements) {
        std::vector<std::shared_ptr<Edge>> possibleEdges;
        for (auto &other: this->settlements) {
            /* Add edge between <current> and <other> if they are not equal and the distance between them is smaller than the maximum allowed edge distance*/
            if (current->distance(*other) < this->configuration.CONNECT_DISTANCE and current != other) {
                possibleEdges.push_back(std::make_shared<Edge>(current, other, directed));
            }
        }
        std::sort(possibleEdges.begin(), possibleEdges.end(), cmpInverse);
        for (int i = 0; i < std::min((unsigned long)this->configuration.EDGES_PER_NODE,possibleEdges.size()); i++) {
            /*Add the edges to the most important neighbors (max: EdgesPerNode) to the network*/
            this->addEdge(std::move((possibleEdges[i])));
        }
        progress.visit();
    }
}

void Network::contract() {
    contract(this->configuration.mergeStrategy);
}

void Network::contract(const std::unique_ptr<MergeStrategy> &strategy) {
                                // estimated iterations
    auto progress = ProgressPrinter("Contracting Network with "+std::to_string(settlements.size()) + " Settlements and " + std::to_string(edges.size()) +" Edges");

    std::priority_queue<std::shared_ptr<Edge>,std::vector<std::shared_ptr<Edge>>, Edge::Comparator> edgesToRemove;
    //Find edges connecting two settlements with distance smaller than the merge distance
    for (auto &edge: edges) {
        if (edge->getDistance() < configuration.MERGE_DISTANCE) {
            edgesToRemove.push(edge);
        }
    }
    int i = 0;
    /* While there are still edges to contract perform while loop */
    while (not edgesToRemove.empty()) {
        if (i % 500 == 0) {
            std::cout << "Edges to remove: " << edgesToRemove.size() << std::endl;
        }
        i++;
        auto edge = edgesToRemove.top();
        edgesToRemove.pop();
        auto u = edge->getFrom();
        auto v = edge->getTo();
        auto s = std::move(strategy->merge(*u, *v)); //merge settlements according to mergeStrategy
        addSettlement(s);
        removeEdge(edge);
        auto uFormerEdges = getEdgesOfSettlement(u);
        auto vFormerEdges = getEdgesOfSettlement(v);
        /* Store all former of edges of u and v in a list*/
        auto formerEdges = std::vector<std::shared_ptr<Edge>>();
        formerEdges.reserve(uFormerEdges.size() + vFormerEdges.size());
        for (auto &e: uFormerEdges) {
            formerEdges.push_back(e);
        }
        for (auto &e: vFormerEdges) {
            formerEdges.push_back(e);
        }
        EdgeSet newEdges;
        for (auto &formerEdge: formerEdges) {
            if (formerEdge->getFrom() == u or formerEdge->getFrom() == v) {
                /* u or v was the start point of the edge -> s is now the startpoint*/
                newEdges.insert(std::make_shared<Edge>(s, formerEdge->getTo()));
            } else {
                /* u or v was the end point of the edge -> s is now the endpoint*/
                newEdges.insert(std::make_shared<Edge>(formerEdge->getFrom(), s));
            }
        }
        removeSettlement(u);
        removeSettlement(v);
        for (auto &newEdge: newEdges) {
            addEdge(newEdge);
            if (validEdge(newEdge) and newEdge->getDistance() < configuration.MERGE_DISTANCE) {
                /*Push edges to queue if the new edge can be contracted too*/
                edgesToRemove.push(newEdge);
            }
        }
    }
    std::cout << "Edges to remove: " << edgesToRemove.size() << std::endl;
    std::cout << "Merging overlapping Settlements" << std::endl;
    bool merging = true;
    int j = 0;
    /* Merge overlapping settlements*/
    while (merging) {
        merging = false;
        EdgeSet newEdges;
        std::shared_ptr<Settlement> nextMerge;
        for (auto &s1: settlements) {
            for (auto &s2: settlements) {
                /* If s1 and s2 are not equal but overlap or contain each other -> merge s1 and s2*/
                if (s1 != s2 and (s1->getPolygon()->intersects(*s2->getPolygon()) or s1->getPolygon()->contains(*s2->getPolygon()) )) {
                    /* Same procedure as above*/
                    removeEdge(s1, s2, false);
                    nextMerge = strategy->merge(*s1, *s2);
                    auto uFormerEdges = getEdgesOfSettlement(s1);
                    auto vFormerEdges = getEdgesOfSettlement(s2);
                    auto formerEdges = std::vector<std::shared_ptr<Edge>>();
                    formerEdges.reserve(uFormerEdges.size() + vFormerEdges.size());
                    for (auto &e: uFormerEdges) {
                        formerEdges.push_back(e);
                    }
                    for (auto &e: vFormerEdges) {
                        formerEdges.push_back(e);
                    }
                    for (auto &formerEdge: formerEdges) {
                        if (formerEdge->getFrom() == s1 or formerEdge->getFrom() == s2) {
                            newEdges.insert(std::make_shared<Edge>(nextMerge, formerEdge->getTo()));
                        } else {
                            newEdges.insert(std::make_shared<Edge>(formerEdge->getFrom(), nextMerge));
                        }
                    }
                    removeSettlement(s1);
                    removeSettlement(s2);
                    merging = true;
                    break;
                }
            }
            if (merging) {
                break;
            }
        }
        if (merging) {
            addSettlement(nextMerge);
            for (auto &newEdge: newEdges) {
                addEdge(newEdge);
            }
        }
        if (j % 100== 0) {
            std::cout <<"#";
        }
        j++;
    }
    std::cout << std::endl;
}
void Network::setNetworkEntityStrategies() {
    Settlement::setSettlementValueStrategy(this->configuration.settlementValueStrategy);
    Edge::setDefaultWeightFunction(this->configuration.weightStrategy);
}

bool Network::validEdge(const std::shared_ptr<Edge> &edge) {
    return not containsEdge(edge) and containsSettlement(edge->getFrom()) and containsSettlement(edge->getTo());
}




