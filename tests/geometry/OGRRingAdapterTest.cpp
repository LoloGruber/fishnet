#include <gtest/gtest.h>
#include <fishnet/TestUtil.hpp>
#include <fishnet/OGRRingAdapter.hpp>
#include <fishnet/Ring.hpp>
#include <fishnet/Line.hpp>

using namespace fishnet::geometry;
using namespace fishnet::test;

/**
 * The OGRRingAdapter is expected to be substitutable for fishnet::geometry::Ring, so most
 * assertions below are cross checked against the equivalent Ring<double>.
 */
class OGRRingAdapterTest : public ::testing::Test {
protected:
    // Complex ring (same as RingTest)
    inline static const std::vector<Vec2D<double>> complexPoints {
        Vec2D<double>(0,4),
        Vec2D<double>(2,3),
        Vec2D<double>(2,2),
        Vec2D<double>(4,2),
        Vec2D<double>(2,0),
        Vec2D<double>(-2,-1),
        Vec2D<double>(0,1),
        Vec2D<double>(-3,2)
    };
    // Convex ring (same as RingTest)
    inline static const std::vector<Segment<double>> convexSegments {
        Segment<double>(Vec2D<double>(3,4),Vec2D<double>(4,5)),
        Segment<double>(Vec2D<double>(4,5),Vec2D<double>(5,4)),
        Segment<double>(Vec2D<double>(5,4),Vec2D<double>(5,3)),
        Segment<double>(Vec2D<double>(5,3),Vec2D<double>(4,3)),
        Segment<double>(Vec2D<double>(4,3),Vec2D<double>(3,4))
    };
    // Square ring (same as RingTest)
    inline static const std::vector<Vec2D<double>> squarePoints {Vec2D(0,0), Vec2D(0,1), Vec2D(1,1), Vec2D(1,0)};

    void SetUp() override {
        complexRing = std::make_unique<OGRRingAdapter>(expectedComplex());
        convexRing = std::make_unique<OGRRingAdapter>(expectedConvex());
        squareRing = std::make_unique<OGRRingAdapter>(expectedSquare());
    }

    std::unique_ptr<OGRRingAdapter> complexRing;
    std::unique_ptr<OGRRingAdapter> convexRing;
    std::unique_ptr<OGRRingAdapter> squareRing;

    static Ring<double> expectedSquare() {
        static auto ring = Ring<double>(squarePoints);
        return ring;
    }

    static Ring<double> expectedComplex(){
        static auto ring = Ring<double>(complexPoints);
        return ring;
    }

    static Ring<double> expectedConvex(){
        static auto ring = Ring<double>(convexSegments);
        return ring;
    }
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
    OGRRingAdapter adapter(std::move(ogrRing));
    EXPECT_SORTED_RANGE_EQ(adapter.getPoints(), squareRing->getPoints());
}

TEST_F(OGRRingAdapterTest, initFromUnclosedOGRPtr) {
    // an OGR ring which was not closed by its producer must still behave like a closed ring
    OGRLinearRing * ring = static_cast<OGRLinearRing *>(OGRGeometryFactory::createGeometry(wkbLinearRing));
    ring->addPoint(0, 0);
    ring->addPoint(0, 1);
    ring->addPoint(1, 1);
    ring->addPoint(1, 0);
    auto ogrRing = OGRUniquePtr<OGRLinearRing>(ring);
    OGRRingAdapter adapter(std::move(ogrRing));
    EXPECT_SIZE(adapter.getPoints(), 4);
    EXPECT_SIZE(adapter.getSegments(), 4);
    EXPECT_DOUBLE_EQ(adapter.area(), 1.0);
}

TEST_F(OGRRingAdapterTest, copyAndMove) {
    OGRRingAdapter copy(*squareRing);
    EXPECT_EQ(copy, *squareRing);
    OGRRingAdapter moved(std::move(copy));
    EXPECT_EQ(moved, *squareRing);
}

// ============================================================================
// Accessors
// ============================================================================

TEST_F(OGRRingAdapterTest, getPoints) {
    // the adapter reports the points of the canonical form of the ring, which contains the same
    // points as the ring it was built from, though not necessarily starting at the same vertex
    EXPECT_UNSORTED_RANGE_EQ(squareRing->getPoints(), squarePoints);
    EXPECT_UNSORTED_RANGE_EQ(complexRing->getPoints(), complexPoints);
}

TEST_F(OGRRingAdapterTest, pointsAreInCanonicalOrder) {
    // a ring which already starts at its lexicographically smallest vertex is left as it is
    EXPECT_SORTED_RANGE_EQ(squareRing->getPoints(), squarePoints);

    // ... any other ring is rotated to start there, keeping the cyclic order of its points.
    // This is what lets equality ignore the starting vertex while staying a coordinate comparison.
    std::vector<Vec2DReal> canonicalComplex {
        Vec2DReal(-3,2),
        Vec2DReal(0,4),
        Vec2DReal(2,3),
        Vec2DReal(2,2),
        Vec2DReal(4,2),
        Vec2DReal(2,0),
        Vec2DReal(-2,-1),
        Vec2DReal(0,1)
    };
    EXPECT_SORTED_RANGE_EQ(complexRing->getPoints(), canonicalComplex);
}

TEST_F(OGRRingAdapterTest, getPointsMatchesRing) {
    // same vertices as the fishnet ring; the order is the canonical one, @see pointsAreInCanonicalOrder
    EXPECT_UNSORTED_RANGE_EQ(squareRing->getPoints(), expectedSquare().getPoints());
    EXPECT_UNSORTED_RANGE_EQ(complexRing->getPoints(), expectedComplex().getPoints());
    EXPECT_UNSORTED_RANGE_EQ(convexRing->getPoints(), expectedConvex().getPoints());
}

TEST_F(OGRRingAdapterTest, getSegments) {
    EXPECT_UNSORTED_RANGE_EQ(squareRing->getSegments(), expectedSquare().getSegments());
    EXPECT_UNSORTED_RANGE_EQ(complexRing->getSegments(), expectedComplex().getSegments());
    EXPECT_UNSORTED_RANGE_EQ(convexRing->getSegments(), expectedConvex().getSegments());
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
    EXPECT_DOUBLE_EQ(complexRing->area(), 15.5);
    EXPECT_DOUBLE_EQ(convexRing->area(), 2.5);
}

TEST_F(OGRRingAdapterTest, areaMatchesRing) {
    EXPECT_DOUBLE_EQ(squareRing->area(), expectedSquare().area());
    EXPECT_DOUBLE_EQ(complexRing->area(), expectedComplex().area());
    EXPECT_DOUBLE_EQ(convexRing->area(), expectedConvex().area());
}

TEST_F(OGRRingAdapterTest, centroid) {
    // fishnet defines the centroid of a ring as the mean of its points, which for asymmetric rings
    // differs from the area weighted centroid OGR/GEOS computes
    EXPECT_EQ(squareRing->centroid(), Vec2DReal(0.5, 0.5));
    EXPECT_EQ(convexRing->centroid(), Vec2DReal(4.2, 3.8));
    EXPECT_EQ(complexRing->centroid(), Vec2DReal(5.0/8.0, 13.0/8.0));
}

TEST_F(OGRRingAdapterTest, centroidMatchesRing) {
    EXPECT_EQ(squareRing->centroid(), expectedSquare().centroid());
    EXPECT_EQ(complexRing->centroid(), expectedComplex().centroid());
    EXPECT_EQ(convexRing->centroid(), expectedConvex().centroid());
}

TEST_F(OGRRingAdapterTest, aaBB) {
    auto bbox = squareRing->aaBB();
    EXPECT_DOUBLE_EQ(bbox.left(), 0.0);
    EXPECT_DOUBLE_EQ(bbox.right(), 1.0);
    EXPECT_DOUBLE_EQ(bbox.top(), 1.0);
    EXPECT_DOUBLE_EQ(bbox.bottom(), 0.0);

    auto complexBox = complexRing->aaBB();
    EXPECT_DOUBLE_EQ(complexBox.left(), -3.0);
    EXPECT_DOUBLE_EQ(complexBox.right(), 4.0);
    EXPECT_DOUBLE_EQ(complexBox.top(), 4.0);
    EXPECT_DOUBLE_EQ(complexBox.bottom(), -1.0);
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
    EXPECT_TRUE(squareRing->isOnBoundary(Vec2DReal(0.0, 0.0)));  // vertex
    EXPECT_TRUE(squareRing->isOnBoundary(Vec2DReal(0.5, 0.0)));  // on an edge
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

TEST_F(OGRRingAdapterTest, pointLocationIsExhaustive) {
    // every point is either inside, on the boundary or outside - never two of them
    std::vector<Vec2DReal> probes {
        {0.5,0.5},{0,0},{1,1},{0.5,0},{2,2},{-1,-1},{1,0.5},{0.999,0.999}
    };
    for(const auto & probe : probes){
        int locations = int(squareRing->isInside(probe)) + int(squareRing->isOnBoundary(probe)) + int(squareRing->isOutside(probe));
        EXPECT_EQ(locations, 1) << "Point " << probe.toString() << " must have exactly one location";
    }
}

TEST_F(OGRRingAdapterTest, pointLocationMatchesRing) {
    auto reference = expectedSquare();
    std::vector<Vec2DReal> probes {
        {0.5,0.5},{0,0},{1,1},{0.5,0},{2,2},{-1,-1},{1,0.5}
    };
    for(const auto & probe : probes){
        EXPECT_EQ(squareRing->isInside(probe), reference.isInside(probe)) << probe.toString();
        EXPECT_EQ(squareRing->isOnBoundary(probe), reference.isOnBoundary(probe)) << probe.toString();
        EXPECT_EQ(squareRing->isOutside(probe), reference.isOutside(probe)) << probe.toString();
        EXPECT_EQ(squareRing->contains(probe), reference.contains(probe)) << probe.toString();
    }
}

TEST_F(OGRRingAdapterTest, containsPoint) {
    // contains is boundary inclusive
    EXPECT_TRUE(squareRing->contains(Vec2DReal(0.5, 0.5)));
    EXPECT_TRUE(squareRing->contains(Vec2DReal(0.0, 0.0)));     // vertex
    EXPECT_TRUE(squareRing->contains(Vec2DReal(1.0, 1.0)));     // vertex
    EXPECT_TRUE(squareRing->contains(Vec2DReal(0.5, 1.0)));     // edge
    EXPECT_FALSE(squareRing->contains(Vec2DReal(2.0, 2.0)));
}

// ============================================================================
// Linear geometry intersection
// ============================================================================

TEST_F(OGRRingAdapterTest, intersects) {
    EXPECT_TRUE(squareRing->intersects(Line<double>::horizontalLine(0.5)));
    // a line collinear with an edge does not cross the ring
    EXPECT_FALSE(squareRing->intersects(Y_AXIS));
    EXPECT_FALSE(squareRing->intersects(Segment(Vec2DReal(2,2), Vec2DReal(3,3))));
    // segment fully inside does NOT intersect the boundary
    EXPECT_FALSE(squareRing->intersects(Segment(Vec2DReal(0.25,0.25), Vec2DReal(0.75,0.75))));
    // segment sticking out of the ring crosses the boundary
    EXPECT_TRUE(squareRing->intersects(Segment(Vec2DReal(0.5,0.5), Vec2DReal(5,5))));
}

TEST_F(OGRRingAdapterTest, intersectsMatchesRing) {
    auto reference = expectedSquare();
    EXPECT_EQ(squareRing->intersects(Line<double>::horizontalLine(0.5)), reference.intersects(Line<double>::horizontalLine(0.5)));
    EXPECT_EQ(squareRing->intersects(Y_AXIS), reference.intersects(Y_AXIS));
    EXPECT_EQ(squareRing->intersects(Segment(Vec2DReal(2,2), Vec2DReal(3,3))), reference.intersects(Segment(Vec2DReal(2,2), Vec2DReal(3,3))));
    EXPECT_EQ(squareRing->intersects(Segment(Vec2DReal(0.25,0.25), Vec2DReal(0.75,0.75))), reference.intersects(Segment(Vec2DReal(0.25,0.25), Vec2DReal(0.75,0.75))));
    EXPECT_EQ(squareRing->intersects(Segment(Vec2DReal(0.5,0.5), Vec2DReal(5,5))), reference.intersects(Segment(Vec2DReal(0.5,0.5), Vec2DReal(5,5))));
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
    EXPECT_TRUE(squareRing->contains(Segment(Vec2DReal(0.0, 0.0), Vec2DReal(1.0, 0.0))));    // along an edge
    EXPECT_TRUE(squareRing->contains(Segment(Vec2DReal(0.5, 0.5), Vec2DReal(0.5, 0.5))));    // degenerate, inside
    EXPECT_TRUE(squareRing->contains(Segment(Vec2DReal(0.1, 0.1), Vec2DReal(0.9, 0.9))));    // fully inside
    EXPECT_TRUE(squareRing->contains(Segment(Vec2DReal(0.0, 0.0), Vec2DReal(1.0, 1.0))));    // diagonal, vertex to vertex
    EXPECT_FALSE(squareRing->contains(Segment(Vec2DReal(-1.0, 0.5), Vec2DReal(2.0, 0.5))));  // crossing through
    EXPECT_FALSE(squareRing->contains(Segment(Vec2DReal(2.0, 2.0), Vec2DReal(3.0, 3.0))));   // fully outside
    EXPECT_FALSE(squareRing->contains(Segment(Vec2DReal(0.5, 0.5), Vec2DReal(2.0, 0.5))));   // half outside
}

TEST_F(OGRRingAdapterTest, containsRing) {
    // a ring contains itself
    EXPECT_TRUE(squareRing->contains(*squareRing));
    // an identical ring of another IRing implementation
    EXPECT_TRUE(squareRing->contains(expectedSquare()));
    // a strictly smaller ring
    Ring<double> inner(std::vector<Vec2DReal>{{0.25,0.25},{0.25,0.75},{0.75,0.75},{0.75,0.25}});
    EXPECT_TRUE(squareRing->contains(inner));
    EXPECT_FALSE(OGRRingAdapter(inner).contains(*squareRing));
    // a ring sharing only a corner is not contained
    Ring<int> touching(std::vector<Vec2D<int>>{Vec2D(1, 1), Vec2D(2, 1), Vec2D(2, 2), Vec2D(1, 2)});
    EXPECT_FALSE(squareRing->contains(touching));
    // a disjoint ring is not contained
    Ring<int> disjoint(std::vector<Vec2D<int>>{Vec2D(5, 5), Vec2D(6, 5), Vec2D(6, 6), Vec2D(5, 6)});
    EXPECT_FALSE(squareRing->contains(disjoint));
}

TEST_F(OGRRingAdapterTest, containsRingMatchesRing) {
    auto reference = expectedSquare();
    Ring<double> inner(std::vector<Vec2DReal>{{0.25,0.25},{0.25,0.75},{0.75,0.75},{0.75,0.25}});
    Ring<int> touching(std::vector<Vec2D<int>>{Vec2D(1, 1), Vec2D(2, 1), Vec2D(2, 2), Vec2D(1, 2)});
    Ring<int> disjoint(std::vector<Vec2D<int>>{Vec2D(5, 5), Vec2D(6, 5), Vec2D(6, 6), Vec2D(5, 6)});
    EXPECT_EQ(squareRing->contains(inner), reference.contains(inner));
    EXPECT_EQ(squareRing->contains(touching), reference.contains(touching));
    EXPECT_EQ(squareRing->contains(disjoint), reference.contains(disjoint));
}

// ============================================================================
// Ring spatial relations
// ============================================================================

TEST_F(OGRRingAdapterTest, crosses) {
    Ring<int> crossing(std::vector<Vec2D<int>>{Vec2D(0, 2), Vec2D(2, 2), Vec2D(2, -1), Vec2D(0, -1)});
    EXPECT_TRUE(squareRing->crosses(crossing));
    EXPECT_EQ(squareRing->crosses(crossing), expectedSquare().crosses(crossing));

    Ring<int> disjoint(std::vector<Vec2D<int>>{Vec2D(2, 2), Vec2D(3, 2), Vec2D(3, 3), Vec2D(2, 3)});
    EXPECT_FALSE(squareRing->crosses(disjoint));
    EXPECT_EQ(squareRing->crosses(disjoint), expectedSquare().crosses(disjoint));

    // a ring does not cross itself
    EXPECT_FALSE(squareRing->crosses(*squareRing));
    EXPECT_FALSE(squareRing->crosses(expectedSquare()));

    // a ring sharing only a corner does not cross
    Ring<int> touching(std::vector<Vec2D<int>>{Vec2D(1, 1), Vec2D(2, 1), Vec2D(2, 2), Vec2D(1, 2)});
    EXPECT_FALSE(squareRing->crosses(touching));
    EXPECT_EQ(squareRing->crosses(touching), expectedSquare().crosses(touching));
}

TEST_F(OGRRingAdapterTest, touches) {
    // sharing the corner (1,1)
    Ring<int> touching(std::vector<Vec2D<int>>{Vec2D(1, 1), Vec2D(2, 1), Vec2D(2, 2), Vec2D(1, 2)});
    EXPECT_TRUE(squareRing->touches(touching));
    EXPECT_EQ(squareRing->touches(touching), expectedSquare().touches(touching));

    // sharing the whole edge x=1
    Ring<int> edgeNeighbour(std::vector<Vec2D<int>>{Vec2D(1, 0), Vec2D(2, 0), Vec2D(2, 1), Vec2D(1, 1)});
    EXPECT_EQ(squareRing->touches(edgeNeighbour), expectedSquare().touches(edgeNeighbour));

    Ring<int> disjoint(std::vector<Vec2D<int>>{Vec2D(2, 2), Vec2D(3, 2), Vec2D(3, 3), Vec2D(2, 3)});
    EXPECT_FALSE(squareRing->touches(disjoint));

    // a ring does not touch itself
    EXPECT_FALSE(squareRing->touches(*squareRing));
}

TEST_F(OGRRingAdapterTest, distance) {
    // rings which contain each other or cross have no gap between them
    EXPECT_DOUBLE_EQ(squareRing->distance(*squareRing), 0.0); // a ring has no gap to itself
    EXPECT_DOUBLE_EQ(squareRing->distance(expectedSquare()), expectedSquare().distance(expectedSquare()));

    // touching rings have distance 0
    Ring<int> touching(std::vector<Vec2D<int>>{Vec2D(1, 1), Vec2D(2, 1), Vec2D(2, 2), Vec2D(1, 2)});
    EXPECT_DOUBLE_EQ(squareRing->distance(touching), 0.0);
    EXPECT_DOUBLE_EQ(squareRing->distance(touching), expectedSquare().distance(touching));

    // disjoint rings report their gap
    Ring<int> disjoint(std::vector<Vec2D<int>>{Vec2D(2, 0), Vec2D(3, 0), Vec2D(3, 1), Vec2D(2, 1)});
    EXPECT_DOUBLE_EQ(squareRing->distance(disjoint), 1.0);
    EXPECT_DOUBLE_EQ(squareRing->distance(disjoint), expectedSquare().distance(disjoint));

    // same result whether the argument is OGR backed or not
    EXPECT_DOUBLE_EQ(squareRing->distance(OGRRingAdapter(disjoint)), squareRing->distance(disjoint));
}

// ============================================================================
// Base class behavior
// ============================================================================

TEST_F(OGRRingAdapterTest, equalityAndHash) {
    EXPECT_EQ(*squareRing, *squareRing);
    EXPECT_EQ(complexRing->hash(), complexRing->hash());

    OGRRingAdapter copy(*squareRing);
    EXPECT_EQ(*squareRing, copy);
    EXPECT_EQ(squareRing->hash(), copy.hash());
    EXPECT_NE(*squareRing, *complexRing);
}

TEST_F(OGRRingAdapterTest, equalityIsIndependentOfStartingVertexAndWindingOrder) {
    // same square, but starting at another vertex
    OGRRingAdapter rotated(Ring<int>(std::vector<Vec2D<int>>{Vec2D(1,1), Vec2D(1,0), Vec2D(0,0), Vec2D(0,1)}));
    EXPECT_EQ(*squareRing, rotated);
    // same square, but wound the other way round
    OGRRingAdapter reversed(Ring<int>(std::vector<Vec2D<int>>{Vec2D(0,0), Vec2D(1,0), Vec2D(1,1), Vec2D(0,1)}));
    EXPECT_EQ(*squareRing, reversed);
}

TEST_F(OGRRingAdapterTest, equalRingsHashEqually) {
    // the contract which canonicalising the wrapped geometry is there to uphold: rings which
    // compare equal must hash equally, or they break every unordered container they are put in
    OGRRingAdapter rotated(Ring<int>(std::vector<Vec2D<int>>{Vec2D(1,1), Vec2D(1,0), Vec2D(0,0), Vec2D(0,1)}));
    OGRRingAdapter reversed(Ring<int>(std::vector<Vec2D<int>>{Vec2D(0,0), Vec2D(1,0), Vec2D(1,1), Vec2D(0,1)}));
    ASSERT_EQ(*squareRing, rotated);
    ASSERT_EQ(*squareRing, reversed);
    EXPECT_EQ(squareRing->hash(), rotated.hash());
    EXPECT_EQ(squareRing->hash(), reversed.hash());

    EXPECT_NE(squareRing->hash(), complexRing->hash());
}

TEST_F(OGRRingAdapterTest, toString) {
    auto str = squareRing->toString();
    EXPECT_FALSE(str.empty());
}
