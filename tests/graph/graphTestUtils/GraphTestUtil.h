#ifndef GraphTestUtil_H
#define GraphTestUtil_H

#include "Graph.h"
#include "IDNode.h"
#include "XYNode.h"
#include <random>


static fishnet::graph::DirectedGraph<IDNode> getSimpleDirectedGraph(IDNode & n1, IDNode & n2){
    fishnet::graph::DirectedGraph<IDNode> simple;
    simple.addNode(n1);
    simple.addNode(n2);
    simple.addEdge(n1,n2);
    return simple;
}

static fishnet::graph::UndirectedGraph<IDNode> getSimpleUndirectedGraph(IDNode & n1, IDNode & n2){
    fishnet::graph::UndirectedGraph<IDNode> simple;
    simple.addNode(n1);
    simple.addNode(n2);
    simple.addEdge(n1,n2);
    return simple;
}

static std::vector<XYNode> randomNodes( int amount,XYNode min = XYNode(0,0),XYNode max = XYNode(100,100)) {
    std::vector<XYNode> nodes;
    nodes.reserve(amount);
    double minX = min.getX();
    double maxX = max.getX();
    double minY = min.getY();
    double maxY = max.getY();
    std::uniform_real_distribution<double> xRand(minX,maxX);
    std::uniform_real_distribution<double> yRand(minY,maxY);
    std::default_random_engine re;
    for(int i = 0; i < amount; i++){
        double x = xRand(re);
        double y = yRand(re);
        nodes.push_back(XYNode(x,y));
    }
    return nodes;
}


static std::vector<IDNode> getVectorOfNodes(size_t amount){
    std::vector<IDNode> nodes;
    nodes.reserve(amount);
    for(size_t i = 0; i< amount; i++) {
        nodes.emplace_back(IDNode());
    }
    return nodes;
}


static fishnet::graph::UndirectedGraph<IDNode> getStarGraph( IDNode & center, size_t amountOfOtherNodes){
    fishnet::graph::UndirectedGraph<IDNode> g;
    g.addNode(center);
    auto otherNodes = getVectorOfNodes(amountOfOtherNodes);
    g.addNodes(otherNodes);
    for(auto & node: otherNodes){
        g.addEdge(center,node);
    }
    return g;
}


static fishnet::graph::UndirectedGraph<IDNode> getCompleteIDGraph(size_t amountOfNodes){
    std::vector<IDNode> nodes;
    nodes.reserve(amountOfNodes);
    for(size_t i = 0; i < amountOfNodes; i++ ) {
        nodes.emplace_back(IDNode());
    }
    fishnet::graph::UndirectedGraph<IDNode> g(nodes);
    for( auto & node: nodes) {
        for( auto & other: nodes){
            if(node != other){
                g.addEdge(node,other);
            }
        }
    }
    return g;
}


static fishnet::graph::UndirectedGraph<XYNode> getCompleteXYGraph(std::vector<XYNode> & nodes){
    fishnet::graph::UndirectedGraph<XYNode> g;
    g.addNodes(nodes);
    for (auto & node: nodes){
        for (auto & other: nodes){
            if(node != other){
                g.addEdge(node,other);
            }
        }
    }
    return g;
}


#endif

