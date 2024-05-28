#pragma once
#include "GraphDecorator.hpp"

namespace fishnet::graph::__impl{

template<class G> requires G::edge_type
class DirectedAcyclicGraph: protected GraphDecorator<DirectedAcyclicGraph<G>,G>{


};

}