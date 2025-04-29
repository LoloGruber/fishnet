//
// Created by grube on 04.01.2022.
//

#ifndef BACHELORARBEIT_WEIGHT_H
#define BACHELORARBEIT_WEIGHT_H
#include "Edge.h"
class Edge;

/**
 * Abstract class to define the behavior of edge weights
 * Derived classes have to implement <accept>, <fieldname> and <fieldtype> method
 */
template<T,A>
class Weight {
public:
    A accept(Edge & edge) = 0;
};



#endif //BACHELORARBEIT_WEIGHT_H
