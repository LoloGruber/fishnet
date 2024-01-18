#include <gtest/gtest.h>
#include "Normalize.hpp"
#include "Constants.hpp"
#include "Testutil.h"

using namespace fishnet::math;

TEST(NormalizeTest, Normalize2PI){
    std::vector<double> testValues = {
        0.0,3.14,3.15,-3.14,6.3,-6.3,PI,TWO_PI
    };
    std::vector<double> results;
    for(auto v : testValues){
        results.push_back(normalize(v,TWO_PI));
    }
    for(auto v : results){
        testutil::EXPECT_IN_RANGE(v,0,TWO_PI);
    }
    EXPECT_EQ(normalize(PI,TWO_PI),PI);
    EXPECT_EQ(normalize(0,TWO_PI),0);
    EXPECT_EQ(normalize(TWO_PI,TWO_PI),0);
}

TEST(NormalizeTest, NormalizePlusMinusPI){
    std::vector<double> testValues = {
        0.0,3.14,3.15,-3.14,6.3,-6.3,PI,TWO_PI
    };
    std::vector<double> results;
    for(auto v : testValues){
        results.push_back(normalize(v,-PI,PI));
    }
    for(auto v : results){
        testutil::EXPECT_IN_RANGE(v,-PI,PI);
    }
    EXPECT_EQ(normalize(0,-PI,PI),0);
    EXPECT_EQ(normalize(-PI,-PI,PI),-PI);
    EXPECT_EQ(normalize(PI,-PI,PI),-PI);
    EXPECT_EQ(normalize(TWO_PI,-PI,PI),0);
}

TEST(NormalizeTest, NormalizeDegree){
    std::vector<double> testValues = {
        0.0, -180.0,90.0,360.0,720.0,359.0,361.0,-0.1
    };
    for(auto v: testValues){
        testutil::EXPECT_IN_RANGE(normalize(v,360.0),0.0,360.0);
    }
    EXPECT_EQ(normalize(360.0,360.0),0);
    EXPECT_EQ(normalize(90.0,360.0),90.0);
    EXPECT_EQ(normalize(-180.0,360.0),180.0);
}




