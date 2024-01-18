//
// Created by grube on 25.11.2021.
//

#ifndef BACHELORARBEIT_NETWORK_H
#define BACHELORARBEIT_NETWORK_H
#include <list>
#include <memory>
#include <algorithm>
#include <unordered_set>
#include "NodeValue.h"
#include "Edge.h"
#include "Weight.h"
#include "NetworkConfiguration.h"
#include "Referential.h"
#include "EmptyWeightFunction.h"

/**
 * Class responsible for the Network itself
 * Stores nodes and edges, as well as the configuration parameters of the network
 */
template<Node,class A>
class Network {
public:
    /* Define abbreviation for the set of settlements and the set of edges*/
    using SettlementSet = std::unordered_set<std::shared_ptr<Node>,Referential::Hash, Referential::Equal>;
    using EdgeSet = std::unordered_set<std::shared_ptr<Edge<T,A>,Referential::Hash, Referential::Equal>;
private:
    SettlementSet settlements = SettlementSet();
    EdgeSet edges = EdgeSet();
    struct NetworkConfiguration configuration = NetworkConfiguration();

    /**
     * Helper method to determine if the given edge is valid for insertion
     * @param edge to test
     * @return whether edge is not already stored and if both endpoints are contained in the set of settlements
     */
    bool validEdge(const std::shared_ptr<Edge> &edge);
public:

    Network(){
        setNetworkEntityStrategies();
    }

    explicit Network(SettlementSet  &settlements){
        this->settlements = std::move(settlements);
        setNetworkEntityStrategies();
    }

    explicit Network(std::vector<std::shared_ptr<Settlement>> & settlements){
        for (auto &s: settlements) {
            this->settlements.insert(std::move(s));
        }
        setNetworkEntityStrategies();
    }

    explicit Network(NetworkConfiguration &graphConfiguration){
        this->settlements = SettlementSet ();
        this->configuration = std::move(graphConfiguration);
        setNetworkEntityStrategies();
    }

    Network(SettlementSet &settlements, NetworkConfiguration & graphConfiguration ){
        this->settlements = std::move(settlements);
        this->configuration = std::move( graphConfiguration);
        setNetworkEntityStrategies();
    }

    Network(std::vector<std::shared_ptr<Settlement>> & settlements, NetworkConfiguration & graphConfiguration ){
        this->configuration = std::move( graphConfiguration);
        for (auto &s: settlements) {
            this->settlements.insert(std::move(s));
        }
        setNetworkEntityStrategies();
    }
    [[nodiscard]] const SettlementSet &getSettlements() const;

    const EdgeSet & getEdges() const;

    void addSettlement(const std::shared_ptr<Settlement> & settlement);

    bool containsSettlement(const std::shared_ptr<Settlement> & settlement);

    void removeSettlement(const std::shared_ptr<Settlement> & settlement);

    void addEdge(std::shared_ptr<Settlement> &from, std::shared_ptr<Settlement>&to, bool directed = false);

    void addEdge(std::shared_ptr<Edge> edge);

    void addEdges(std::vector<std::shared_ptr<Edge>> &newEdges);

    bool containsEdge(std::shared_ptr<Settlement> &from, std::shared_ptr<Settlement> &to, bool directed);

    bool containsEdge(const std::shared_ptr<Edge> &edge);

    void removeEdge(const std::shared_ptr<Settlement> &from,const  std::shared_ptr<Settlement> &to, bool directed = false);

    void removeEdge(const std::shared_ptr<Edge> & edge);

    /**
     *
     * @param settlement to find the edges
     * @return all edges involving the current settlement
     *(directed vs undirected is handled by the Edge class)
     */
    std::vector<std::shared_ptr<Edge>> getEdgesOfSettlement(const std::shared_ptr<Settlement> & settlement);

    /**
     * Create the edges to the most influential neighbors
     * @param directed defaults to false, true for directed edges
     */
    void createEdges(bool directed=false);

    /**
     * Overload to call contract(DEFAULTStrategy)
     */
    void contract();

    /**
     * Contracts all edges covering a distance smaller than the merge distance
     * The involved settlemetns are merged, with their old edges removed and new edges to
     * the former neighbors constructed from the combined settlement
     * @param strategy defines merging behavior of settlements
     */
    void contract(const std::unique_ptr<MergeStrategy> & strategy);
};


#endif //BACHELORARBEIT_NETWORK_H
