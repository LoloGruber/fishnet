#pragma once
#include <fishnet/GraphFactory.hpp>
#include <JobAdjacency.hpp>

using JobDAG_t = decltype(fishnet::graph::GraphFactory::DAG<Job>(JobAdjacency(MemgraphConnection::create("",0).value(),0)));

static JobDAG_t loadDAG(JobAdjacency && adj) {
    return fishnet::graph::GraphFactory::DAG<Job>(std::move(adj));
}