#include <gtest/gtest.h>
#include <fishnet/TestUtil.hpp>
#include <fishnet/OGRRingAdapter.hpp>
#include <fishnet/Ring.hpp>
#include <fishnet/Line.hpp>

using namespace fishnet::geometry;
using namespace testutil;

class OGRRingAdapterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Complex ring (same as RingTest)
        std::vector<Vec2D<double>> complexPoints {
            Vec2D<double>(0,4),
            Vec2D<double>(2,3),
            Vec2D<double>(2,2),
            Vec2D<double>(4,2),
            Vec2D<double>(2,0),
            Vec2D<double>(-2,-1),
            Vec2D<double>(0,1),
            Vec2D<double>(-3,2)
        };
        complexRing = std::make_unique<OGRRingAdapter>(Ring<double>(complexPoints));

        // Convex ring (same as RingTest)
        std::vector<Segment<double>> convexSegments {
            Segment<double>(Vec2D<double>(3,4),Vec2D<double>(4,5)),
            Segment<double>(Vec2D<double>(4,5),Vec2D<double>(5,4)),
            Segment<double>(Vec2D<double>(5,4),Vec2D<double>(5,3)),
            Segment<double>(Vec2D<double>(5,3),Vec2D<double>(4,3)),
            Segment<double>(Vec2D<double>(4,3),Vec2D<double>(3,4))
        };
        convexRing = std::make_unique<OGRRingAdapter>(Ring<double>(convexSegments));

        // Unit square
        squareRing = std::make_unique<OGRRingAdapter>(
            Ring<int>(std::vector<Vec2D<int>>{Vec2D(0,0), Vec2D(0,1), Vec2D(1,1), Vec2D(1,0)})
        );
    }

    std::unique_ptr<OGRRingAdapter> complexRing;
    std::unique_ptr<OGRRingAdapter> convexRing;
    std::unique_ptr<OGRRingAdapter> squareRing;
};

// ============================================================================
// Construction
// ============================================================================

TEST_F(OGRRingAdapterTest, initFromRing) {
    EXPECT_NO_FATAL_FAILURE(OGRRingAdapter adapted(*squareRing));
    EXPECT_NO_FATAL_FAILURE(OGRRingAdapter adapted(*complexRing));
    EXPECT_NO_FATAL_FAILURE(OGRRingAdapter adapted(*convexRing));
}

TEST_F(OGRRingAdapterTest, initFromOGRPtr) {
    OGRLinearRing * ring = static_cast<OGRLinearRing *>(OGRGeometryFactory::createGeometry(wkbLinearRing));
    ring->addPoint(0, 0);
    ring->addPoint(0, 1);
    ring->addPoint(1, 1);
    ring->addPoint(1, 0);
    ring->addPoint(0, 0);  // close the ring
    auto ogrRing = OGRUniquePtr<OGRLinearRing>(ring);
    EXPECT_NO_FATAL_FAILURE(OGRRingAdapter adapter(std::move(ogrRing)));
}

// ============================================================================
// Accessors
// ============================================================================

TEST_F(OGRRingAdapterTest, getPoints) {
    std::vector<Vec2DReal> expected {
        Vec2DReal(0,0), Vec2DReal(0,1), Vec2DReal(1,1), Vec2DReal(1,0), Vec2DReal(0,0)
    };
    EXPECT_RANGE_EQ(squareRing->getPoints(), expected);
}

TEST_F(OGRRingAdapterTest, getSegments) {
    std::vector<Segment<double>> expected {
        Segment<double>(Vec2DReal(0,0), Vec2DReal(0,1)),
        Segment<double>(Vec2DReal(0,1), Vec2DReal(1,1)),
        Segment<double>(Vec2DReal(1,1), Vec2DReal(1,0)),
        Segment<double>(Vec2DReal(1,0), Vec2DReal(0,0))
    };
    EXPECT_RANGE_EQ(squareRing->getSegments(), expected);
}

TEST_F(OGRRingAdapterTest, getBoundary) {
    const auto& boundary = squareRing->getBoundary();
    EXPECT_EQ(&boundary, squareRing.get());
}

TEST_F(OGRRingAdapterTest, getHoles) {
    EXPECT_EMPTY(squareRing->getHoles());
    EXPECT_EMPTY(complexRing->getHoles());
    EXPECT_EMPTY(convexRing->getHoles());
}

// ============================================================================
// Geometry properties
// ============================================================================

TEST_F(OGRRingAdapterTest, area) {
    EXPECT_DOUBLE_EQ(squareRing->area(), 1.0);
    EXPECT_DOUBLE_EQ(complexRing->area(),15.5);
    EXPECT_DOUBLE_EQ(convexRing->area(),2.5);
}

TEST_F(OGRRingAdapterTest, centroid) {
    EXPECT_EQ(squareRing->centroid(), Vec2DReal(0.5, 0.5));
    EXPECT_EQ(convexRing->centroid(),Vec2D(4.2,3.8));
    EXPECT_EQ(complexRing->centroid(),Vec2D(5.0/8.0,13.0/8.0));    
}

TEST_F(OGRRingAdapterTest, aaBB) {
    auto bbox = squareRing->aaBB();
    EXPECT_DOUBLE_EQ(bbox.left(), 0.0);
    EXPECT_DOUBLE_EQ(bbox.right(), 1.0);
    EXPECT_DOUBLE_EQ(bbox.top(), 1.0);
    EXPECT_DOUBLE_EQ(bbox.bottom(), 0.0);
}

// ============================================================================
// Point containment
// ============================================================================

TEST_F(OGRRingAdapterTest, isInside) {
    EXPECT_TRUE(squareRing->isInside(Vec2DReal(0.5, 0.5)));
    EXPECT_FALSE(squareRing->isInside(Vec2DReal(0.0, 0.0)));    // on boundary
    EXPECT_FALSE(squareRing->isInside(Vec2DReal(0.5, 0.0)));    // on boundary
    EXPECT_FALSE(squareRing->isInside(Vec2DReal(2.0, 2.0)));    // outside
}

TEST_F(OGRRingAdapterTest, isOnBoundary) {
    EXPECT_TRUE(squareRing->isOnBoundary(Vec2DReal(0.0, 0.0)));
    EXPECT_TRUE(squareRing->isOnBoundary(Vec2DReal(0.5, 0.0)));
    EXPECT_TRUE(squareRing->isOnBoundary(Vec2DReal(1.0, 0.5)));
    EXPECT_FALSE(squareRing->isOnBoundary(Vec2DReal(0.5, 0.5))); // inside
    EXPECT_FALSE(squareRing->isOnBoundary(Vec2DReal(2.0, 2.0))); // outside
}

TEST_F(OGRRingAdapterTest, isOutside) {
    EXPECT_TRUE(squareRing->isOutside(Vec2DReal(2.0, 2.0)));
    EXPECT_TRUE(squareRing->isOutside(Vec2DReal(-1.0, 0.5)));
    EXPECT_FALSE(squareRing->isOutside(Vec2DReal(0.5, 0.5)));   // inside
    EXPECT_FALSE(squareRing->isOutside(Vec2DReal(0.0, 0.0)));   // on boundary
}

TEST_F(OGRRingAdapterTest, containsPoint) {
    EXPECT_TRUE(squareRing->contains(Vec2DReal(0.5, 0.5)));
    EXPECT_TRUE(squareRing->contains(Vec2DReal(0.0, 0.0)));     // boundary counts
    EXPECT_TRUE(squareRing->contains(Vec2DReal(1.0, 1.0)));     // boundary counts
    EXPECT_FALSE(squareRing->contains(Vec2DReal(2.0, 2.0)));
}

// ============================================================================
// Linear geometry intersection
// ============================================================================

TEST_F(OGRRingAdapterTest, intersects) {
    EXPECT_TRUE(squareRing->intersects(Line<double>::horizontalLine(0.5)));
    EXPECT_TRUE(squareRing->intersects(Y_AXIS));
    EXPECT_FALSE(squareRing->intersects(Segment(Vec2DReal(2,2), Vec2DReal(3,3))));
    // segment fully inside does NOT intersect boundary (within ring)
    EXPECT_FALSE(squareRing->intersects(Segment(Vec2DReal(0.25,0.25), Vec2DReal(0.75,0.75))));
}

TEST_F(OGRRingAdapterTest, intersections) {
    auto result = squareRing->intersections(Line<double>::horizontalLine(0.5));
    EXPECT_SIZE(result, 2);
    EXPECT_CONTAINS(result, Vec2DReal(0.0, 0.5));
    EXPECT_CONTAINS(result, Vec2DReal(1.0, 0.5));

    auto yAxisResult = squareRing->intersections(Y_AXIS);
    EXPECT_SIZE(yAxisResult, 2);
    EXPECT_CONTAINS(yAxisResult, Vec2DReal(0.0, 0.0));
    EXPECT_CONTAINS(yAxisResult, Vec2DReal(0.0, 1.0));

    EXPECT_EMPTY(squareRing->intersections(Segment(Vec2DReal(2,2), Vec2DReal(3,3))));
}

// ============================================================================
// Segment and Ring containment
// ============================================================================

TEST_F(OGRRingAdapterTest, containsSegment) {
    EXPECT_TRUE(squareRing->contains(Segment(Vec2DReal(0.0, 0.0), Vec2DReal(1.0, 0.0))));
    EXPECT_TRUE(squareRing->contains(Segment(Vec2DReal(0.5, 0.5), Vec2DReal(0.5, 0.5))));
    EXPECT_FALSE(squareRing->contains(Segment(Vec2DReal(-1.0, 0.5), Vec2DReal(2.0, 0.5))));
    EXPECT_FALSE(squareRing->contains(Segment(Vec2DReal(2.0, 2.0), Vec2DReal(3.0, 3.0))));
}

TEST_F(OGRRingAdapterTest, containsRing) {
    // A ring contains itself (same geometry)
    EXPECT_TRUE(squareRing->contains(*squareRing));

    // Smaller ring inside square
    Ring<int> innerRing(std::vector<Vec2D<int>>{
        Vec2D(0,0), Vec2D(0,1), Vec2D(1,1), Vec2D(1,0)
    });
    // squareRing and innerRing have same boundary, so they contain each other per OGR
    EXPECT_TRUE(squareRing->contains(innerRing));
}

// ============================================================================
// Ring spatial relations
// ============================================================================

TEST_F(OGRRingAdapterTest, crosses) {
    // Ring crossing the square
    Ring<int> crossing(std::vector<Vec2D<int>>{
        Vec2D(0, 2), Vec2D(2, 2), Vec2D(2, -1), Vec2D(0, -1)
    });
    EXPECT_TRUE(squareRing->crosses(crossing));

    // Disjoint ring
    Ring<int> disjoint(std::vector<Vec2D<int>>{
        Vec2D(2, 2), Vec2D(3, 2), Vec2D(3, 3), Vec2D(2, 3)
    });
    EXPECT_FALSE(squareRing->crosses(disjoint));

    // Same ring does not cross itself (OGR semantics)
    EXPECT_FALSE(squareRing->crosses(*squareRing));
}

TEST_F(OGRRingAdapterTest, touches) {
    // Ring touching the square at corner (1,1)
    Ring<int> touching(std::vector<Vec2D<int>>{
        Vec2D(1, 1), Vec2D(2, 1), Vec2D(2, 2), Vec2D(1, 2)
    });
    EXPECT_TRUE(squareRing->touches(touching));

    // Disjoint ring
    Ring<int> disjoint(std::vector<Vec2D<int>>{
        Vec2D(2, 2), Vec2D(3, 2), Vec2D(3, 3), Vec2D(2, 3)
    });
    EXPECT_FALSE(squareRing->touches(disjoint));

    // Same ring does not touch itself
    EXPECT_FALSE(squareRing->touches(*squareRing));
}

TEST_F(OGRRingAdapterTest, distance) {
    // Same ring
    EXPECT_DOUBLE_EQ(squareRing->distance(*squareRing), 0.0);

    // Touching ring -> distance 0
    Ring<int> touching(std::vector<Vec2D<int>>{
        Vec2D(1, 1), Vec2D(2, 1), Vec2D(2, 2), Vec2D(1, 2)
    });
    EXPECT_DOUBLE_EQ(squareRing->distance(touching), 0.0);

    // Disjoint ring
    Ring<int> disjoint(std::vector<Vec2D<int>>{
        Vec2D(2, 0), Vec2D(3, 0), Vec2D(3, 1), Vec2D(2, 1)
    });
    EXPECT_DOUBLE_EQ(squareRing->distance(disjoint), 1.0);
}

// ============================================================================
// Base class behavior
// ============================================================================

TEST_F(OGRRingAdapterTest, equality) {
    // Same ring is equal to itself
    EXPECT_EQ(*squareRing, *squareRing);

    // Two adapters wrapping same geometry
    OGRRingAdapter copy(*squareRing);
    EXPECT_EQ(*squareRing, copy);

    // Two adapters wrapping different geometries
    EXPECT_NE(*squareRing, *complexRing);
}

TEST_F(OGRRingAdapterTest, toString) {
    auto str = squareRing->toString();
    // OGR WKT for a simple ring
    EXPECT_FALSE(str.empty());
    // Should contain "LINEARRING" in WKT
    EXPECT_NE(str.find("LINEARRING"), std::string::npos);
}