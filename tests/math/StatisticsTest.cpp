#include "fishnet/Statistics.hpp"
#include <fishnet/TestUtil.hpp>
#include <gtest/gtest.h>
#include <vector>

using namespace testutil; 

struct Node {
    double value;
};

static std::vector<double> gaussian_distribution(double mean, double stddev, size_t size) {
    assert(size % 2 != 0); // Ensure size is odd for symmetry
    assert(size >= 1);
    std::vector<double> distribution(size);
    size_t center = size / 2;
    distribution[center] = mean;

    double step = stddev * std::sqrt(3.0 / (static_cast<double>(center) * static_cast<double>(center + 1)));
    for (size_t i = 1; i <= center; ++i) {
        distribution[center + i] = mean + step * static_cast<double>(i);
        distribution[center - i] = mean - step * static_cast<double>(i);
    }
    return distribution;
}

class StatisticsTest : public ::testing::Test {
protected:
    const std::vector<double> normalDistribution = gaussian_distribution(0.0, 1.0, 999);

    const std::vector<double> constantDistribution {
            5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0
    };

    const std::vector<Node> nodeDistribution {
            {1.0}, {2.0}, {3.0}, {4.0}, {5.0}
    };

    const std::vector<double> EMPTY {};
};

TEST_F(StatisticsTest, mean) {
    auto meanConstant = fishnet::math::mean(constantDistribution);
    EXPECT_VALUE(meanConstant);
    EXPECT_DOUBLE_EQ(5.0, meanConstant.value());
    EXPECT_NEAR(0.0, fishnet::math::mean(normalDistribution).value(), 1e-10);
    EXPECT_DOUBLE_EQ(3.0, fishnet::math::mean(nodeDistribution, [](const Node& n) { return n.value; }).value());
    EXPECT_EMPTY(fishnet::math::mean(EMPTY));
}

TEST_F(StatisticsTest, var) {
    EXPECT_NEAR(1.0, fishnet::math::var(normalDistribution).value(), 1e-10);
    EXPECT_DOUBLE_EQ(0.0, fishnet::math::var(constantDistribution).value());
    EXPECT_DOUBLE_EQ(2.0, fishnet::math::var(nodeDistribution, [](const Node& n) { return n.value; }).value());
    EXPECT_EMPTY(fishnet::math::var(EMPTY));
}

TEST_F(StatisticsTest, std) {
    EXPECT_NEAR(1.0, fishnet::math::std(normalDistribution).value(), 1e-10);
    EXPECT_DOUBLE_EQ(0.0, fishnet::math::std(constantDistribution).value());
    EXPECT_NEAR(std::sqrt(2.0), fishnet::math::std(nodeDistribution, [](const Node& n) { return n.value; }).value(), 1e-10);
    EXPECT_EMPTY(fishnet::math::std(EMPTY));
}