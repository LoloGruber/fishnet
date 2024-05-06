#include <gtest/gtest.h>
#include <fishnet/Graph.hpp>
#include <fishnet/BFSAlgorithm.hpp>
#include "Testutil.h"
#include "IDNode.h"
#include "GraphTestUtil.h"

#include "XYNode.h"
#include <math.h>
#include <future>

static_assert(fishnet::util::BiPredicate<fishnet::util::TrueBiPredicate,XYNode>);
static_assert(fishnet::graph::NodeBiPredicate<fishnet::util::TrueBiPredicate,XYNode>);

using namespace fishnet::graph;
using namespace fishnet::util;
using namespace testutil;
class ConnectedComponentsTest
: public ::testing::Test{
protected:
    void SetUp() override {
        simple.addEdge(n1,n2);
        xygraph.addEdge(d1,d2);
        xygraph.addEdge(d1,d3);        
        xygraph.addEdge(d1,d4);
        xygraph.addEdge(d2,d5);
        xygraph.addEdge(d4,d5);
        xygraph.addEdge(d4,d6);
        xygraph.addEdge(d5,d6);
        xygraph.addEdge(d5,d7);
        xygraph.addEdge(d8,d9);
        xygraph.addEdge(d9,d10);
        xygraph.addEdge(d10,d11);
        xygraph.addEdge(d9,d11);
        xygraph.addEdge(d11,d12);
    }
    fishnet::graph::UndirectedGraph<IDNode> simple;
    IDNode n1 = IDNode();
    IDNode n2 = IDNode();
    XYNode d1 = XYNode(0,0);    
    XYNode d2 = XYNode(0,1);
    XYNode d3 = XYNode(1,0);
    XYNode d4 = XYNode(1,1);
    XYNode d5 = XYNode(0.5,2);
    XYNode d6 = XYNode(1,2);
    XYNode d7 = XYNode(0,2);
    XYNode d8 = XYNode(3,0);
    XYNode d9 = XYNode(3,-2);
    XYNode d10 = XYNode(3,-2.5);
    XYNode d11 = XYNode(2.75,-2);
    XYNode d12 = XYNode(2.5,-2);
    std::vector<XYNode> xyNodes = {
        d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11,d12
    };    
    fishnet::graph::UndirectedGraph<XYNode> xygraph;

    //Result
    std::vector<XYNode> c0 = {d1};
    std::vector<XYNode> c1 = {d2};
    std::vector<XYNode> c2 = {d3};
    std::vector<XYNode> c3 = {d4};
    std::vector<XYNode> c4 = {d5,d6,d7};
    std::vector<XYNode> c5 = {d8};
    std::vector<XYNode> c6 = {d9,d10,d11,d12};
    std::vector<std::vector<XYNode>> expected = {c0,c1,c2,c3,c4,c5,c6};

    struct DistanceXYNodePredicate{
    double maxDistance = 0.5;
    inline bool operator()(const XYNode & n1, const XYNode & n2)const {
        double deltaX = n1.getX() - n2.getX();
        double deltaY = n1.getY() - n2.getY();
        return sqrt(deltaX*deltaX + deltaY*deltaY) <= maxDistance;
    }
};
};




TEST_F(ConnectedComponentsTest, Instantiation){
    auto res = BFS::connectedComponents(simple);
    auto comp = res.get();
    EXPECT_EQ(comp.size(),1);
    auto compMap = res.asMap();
    EXPECT_EQ(compMap.at(this->n1),compMap.at(this->n2));
}


TEST_F(ConnectedComponentsTest, DistancePredicate){
    auto resMap = BFS::connectedComponents(xygraph,DistanceXYNodePredicate()).asMap();
    std::unordered_set<int> indeces;
    for(auto & v: expected) {
        int currentIndex = resMap.at(v[0]);
        indeces.insert(currentIndex);
        for(auto & n: v) {
            EXPECT_EQ(currentIndex, resMap.at(n));
        }
    }
    EXPECT_EQ(indeces.size(),expected.size());
}

struct XYNodeConsumer{
    void operator()(std::shared_ptr<BlockingQueue<std::pair<int,std::vector<XYNode>>>> q, std::vector<std::vector<XYNode>> & result) {
        auto val = Element(std::pair<int,std::vector<XYNode>>(-1,{}));
        val.get().second.push_back(XYNode(-1,-1));
        while(val != q->getPoisonPill()){
            val = q->take();
            if(not val) {
                q->putPoisonPill();
                return;
            }
            result.push_back(val.get().second);
        }
    }
};



TEST_F(ConnectedComponentsTest, DistancePredicateConcurrent) {
    auto q = std::make_shared<BlockingQueue<std::pair<int,std::vector<XYNode>>>>();
    std::vector<std::vector<XYNode>> actual;
    auto graph = this->xygraph;
    auto future = std::async(std::launch::async, [this,q](){return BFS::connectedComponents(xygraph,q,DistanceXYNodePredicate());});
    XYNodeConsumer c = XYNodeConsumer();
    std::thread xyNodeConsumer(c,q,std::ref(actual));
    auto resMap = future.get().asMap();
    q->putPoisonPill();
    xyNodeConsumer.join();

    std::unordered_set<int> indeces;
    for(auto &v: expected) {
        int currentIndex = resMap.at(v[0]);
        indeces.insert(currentIndex);
        for (auto &n : v) {
            EXPECT_EQ(currentIndex,resMap.at(n));
        }
    }
    EXPECT_EQ(indeces.size(),expected.size());
    EXPECT_EQ(actual.size(),expected.size());
    for(auto &v : actual){
        int indexOfCurrent = resMap.at(v[0]);
        for(auto & node: v) {
            EXPECT_EQ(indexOfCurrent,resMap.at(node));
        }
    }
}

