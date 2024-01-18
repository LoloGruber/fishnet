#include <gtest/gtest.h>
#include "Contraction.h"
#include "WeightedGraph.h"
#include "XYNode.h"
#include "GraphTestUtil.h"
#include "StopWatch.h"
#include <queue>
#include <fstream>


struct MergePredicate{
    bool operator()(const XYNode & n1, const XYNode & n2)const{
        return n1.distanceTo(n2) <= 1.0;
    }
};

 struct MergeFunction{
    XYNode operator()(const XYNode & n1, const XYNode & n2)const {
        return XYNode((n1.getX()+n2.getX())/2,(n1.getY() + n2.getY())/2);
    }
};

struct DistanceFunction{
    double operator()(const XYNode & n1, const XYNode & n2) const {
        return n1.distanceTo(n2);
    }
};

struct EdgeLessThan{
    bool operator()(const fishnet::graph::WeightedEdge auto & e1, const fishnet::graph::WeightedEdge auto & e2) const {
        return e1.getWeight() < e2.getWeight();
    }
};



class ContractionTest: public ::testing::Test{
protected:
    fishnet::graph::Weighted<fishnet::graph::UndirectedGraph<XYNode>,double,DistanceFunction> g;
    std::vector<XYNode> nodes;

    void SetUp() override {
        // Make fully connected graph
        nodes = randomNodes(50);
        g = fishnet::graph::Weighted<fishnet::graph::UndirectedGraph<XYNode>,double,DistanceFunction>(nodes);
        for(auto &from: nodes){
            for(auto & to: nodes ){
                if(from != to) g.addEdge(from,to);
            }
        }
    }

};

TEST_F(ContractionTest, SingleWorker){
    MergePredicate Predicate;
    fishnet::graph::Weighted<fishnet::graph::UndirectedGraph<XYNode>,double,DistanceFunction> result;
    {
        StopWatch watch {"Merging Using Connected Components"};
        result = fishnet::graph::contract(g,MergePredicate(),MergeFunction());
    }
    for(auto & e: result.getEdges()){
        EXPECT_FALSE(Predicate(e.getFrom(),e.getTo()));
    }
}
