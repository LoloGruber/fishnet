//
// Created by grube on 14.01.2022.
//

#ifndef WSF_NETWORK_DART_H
#define WSF_NETWORK_DART_H


#include "Vec2D.hpp"
#include "Vertex.h"
/**
 * Class representing a half edge: <from> -> <to>
 * Identified by its ID
 */
class Dart {
public:
    Vertex from;
    Vertex to;
    std::size_t id;

    Dart() = default;

    Dart(Vertex from, Vertex to, size_t id) : from(from),to(to), id(id){};


    /**
     * @return counterclockwise angle of dart (angle from <to> with reference to <from>)
     */
    double angle() const;

    /**
     *
     * @param other
     * @return whether this.angle() is smaller than other.angle()
     */
    bool operator<(const Dart & other) const;

    /**
     * Equal if id is the same
     */
    bool operator==(const Dart & other)const;

    /**
     * Hash Function is just the id of the Dart
     */
    struct Hash{
        std::size_t operator()(const Dart & obj) const{
            return obj.id;
        };
    };
};


#endif //WSF_NETWORK_DART_H
