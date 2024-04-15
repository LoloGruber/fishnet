#pragma once
#include <fishnet/NetworkConcepts.hpp>
namespace fishnet::graph::__impl{
template<Node N>
struct DefaultBiPredicate{
    inline bool operator()(const N & n1, const N & n2)const {
        return true;
    }
};
}
