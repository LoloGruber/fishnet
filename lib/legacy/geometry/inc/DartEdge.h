//
// Created by grube on 14.01.2022.
//

#ifndef WSF_NETWORK_DARTEDGE_H
#define WSF_NETWORK_DARTEDGE_H
#include "Dart.h"
/**
 * Undirected edge, consisting out of two directed half edges
 */
class DartEdge {
public:
    Dart d1;
    Dart d2;

    DartEdge(Dart u, Dart v): d1(u),d2(v){};

    /**
    * DartEdge is equal to other, if it contains the same darts. d1 and d2 can be flipped
    */
    bool operator==(const DartEdge & other)const;

    double length() const;

    /**
     * Hash Function for a Dart edge
     */
    struct Hash{
        std::size_t operator()(const DartEdge & obj) const{
            std::size_t fromHash = pow((obj.d1.from.id),2);
            std::size_t toHash = pow((obj.d1.to.id),2);
            return fromHash + toHash; // HASH = IDFrom^2 + IDTo^2
        }
    };

    /**
     * DartEdge is equal to other, if it contains the same darts. d1 and d2 can be flipped
     */
    struct Equal{
        bool operator()(const  DartEdge & e1, const DartEdge& e2)const {
            return e1.d1.id == e2.d1.id and e1.d2.id == e2.d2.id or e1.d1.id == e2.d2.id and e2.d2.id == e2.d1.id;
        }
    };
};


#endif //WSF_NETWORK_DARTEDGE_H
