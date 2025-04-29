
// Created by grube on 29.09.2021.
//

#ifndef BACHELORARBEIT_EDGE_H
#define BACHELORARBEIT_EDGE_H
#include <memory>
#include "Weight.h"

class Weight; //forward declaration

/**
 * Class the represents an edge between the two settlements <from> and <to>
 * If edge is directed: [from] -> [to]
 * If edge is undirected: [from] <-> [to]
 *
 * Provides method to instantiate, compare and compute attributes of edges
 */

template<typename T,typename A>
class Edge{
private:
    std::shared_ptr<Node<T>> from;
    std::shared_ptr<Node<T>> to;
    A weight;
    bool directed;

public:
    /**
     * Constructor to create Edge instances
     * @param from
     * @param to
     * @param directed defaults to false
     */
    Edge(const std::shared_ptr<Node<T>> & from,const std::shared_ptr<Node<T>> & to, const A & weight, bool directed=false);

    [[nodiscard]] const std::shared_ptr<Settlement> & getFrom() const;

    [[nodiscard]] const std::shared_ptr<Settlement> & getTo() const;

    [[nodiscard]] bool isDirected() const;

    /**
     *
     * @param other
     * @return True if edges are considered equal: Orientation only matters when the edge is directed
     */
    bool operator == (const Edge<T,A> & other) const;

    /**
     *
     * @param settlement
     * @return whether the given settlement is start or endpoint of this edge
     */
    bool contains(const std::shared_ptr<Node<T>> &node) const;

    /**
     *
     * @return weight according to default weight function (static variable)
     */
    double weigth();
};


#endif //BACHELORARBEIT_EDGE_H
