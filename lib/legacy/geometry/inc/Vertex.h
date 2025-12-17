//
// Created by grube on 15.01.2022.
//

#ifndef WSF_NETWORK_VERTEX_H
#define WSF_NETWORK_VERTEX_H
#include "Vec2D.h"
/**
 * Class storing a vertex with its position and its id
 */
class Vertex {
public:
    std::size_t id;
    Vec2D position;

    Vertex(Vec2D position, std::size_t id): position(position), id(id){};

    /**
     *
     * @param other
     * @return if this.id == other.id
     */
    bool operator==(const Vertex & other)const;

    /**
     * Hash Function just returns the ID
     */
    struct Hash{
        std::size_t operator()(const Vertex & vertex) const{
            return vertex.id;
        }
    };
};


#endif //WSF_NETWORK_VERTEX_H
