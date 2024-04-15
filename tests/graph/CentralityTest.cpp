#include <gtest/gtest.h>
#include <fishnet/Graph.hpp>
#include "Testutil.h"
#include "IDNode.h"
#include "GraphTestUtil.h"
#include <fishnet/DegreeCentrality.hpp>

using namespace fishnet::graph;
using namespace testutil;
class CentralityTest: public ::testing::Test{
protected:
    void SetUp() override {
        this->star = getStarGraph(center,NEIGHBOURS_STAR_GRAPH);
        this->simple = UndirectedGraph<XYNode>();
        this->simple.addNode(n1);
        this->simple.addNode(n2);
        this->simple.addNode(n3);
        this->simple.addEdge(n1,n2);
        this->simple.addEdge(n2,n3);
    }
    static inline size_t NEIGHBOURS_STAR_GRAPH=10;
    IDNode center;
    UndirectedGraph<IDNode> star; 
    XYNode n1 = XYNode(1,1);
    XYNode n2 = XYNode(0,0);
    XYNode n3 = XYNode(3,3);
    UndirectedGraph<XYNode> simple;
};

TEST_F(CentralityTest, DegreeCentralityStarGraph){
    auto result = DegreeCentrality()(star);
    EXPECT_EQ(result.size(),NEIGHBOURS_STAR_GRAPH+1);
    for(const auto &[node,deg]: result){
        if(node==center){
            EXPECT_EQ(deg,NEIGHBOURS_STAR_GRAPH);
        }else {
            EXPECT_EQ(deg,1);
        }
    }
}

TEST_F(CentralityTest, DegreeCentralityChanging){
    auto resultBefore = DegreeCentrality()(simple);
    EXPECT_EQ(resultBefore.size(),simple.getNodes().size());
    for(const auto & [node,deg]:resultBefore) {
        if (node == n1){
            EXPECT_EQ(deg,1);
        }else if (node == n2){
            EXPECT_EQ(deg,2);
 
        }else if (node == n3){
            EXPECT_EQ(deg,1);
        }       
    }
    simple.addEdge(n1,n3);
    auto resultAfter = DegreeCentrality()(simple);
    std::for_each(resultAfter.begin(),resultAfter.end(),[](const auto & pair){EXPECT_EQ(pair.second,2);});
}

TEST_F(CentralityTest, DegreeCentralityCompleteGraph){
    size_t amountNodes = 4;
    auto g = getCompleteIDGraph(amountNodes);
    auto result = DegreeCentrality()(g);
    EXPECT_SIZE(result,amountNodes);
    std::for_each(result.begin(),result.end(),[amountNodes](const auto & pair){EXPECT_EQ(pair.second,amountNodes-1);});

}