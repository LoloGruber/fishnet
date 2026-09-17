#include <gtest/gtest.h>
#include <fishnet/TestUtil.hpp>
#include <fishnet/OGRMultiPolygonAdapter.hpp>
#include <fishnet/MultiPolygon.hpp>
#include <fishnet/Polygon.hpp>
#include <fishnet/Ring.hpp>
#include <fishnet/Line.hpp>

using namespace fishnet::geometry;
using namespace fishnet::test;

/**
 * The OGRMultiPolygonAdapter is expected to be substitutable for fishnet::geometry::MultiPolygon,
 * so most assertions below are cross checked against the equivalent MultiPolygon<Polygon<double>>.
 */
class OGRMultiPolygonAdapterTest : public ::testing::Test {
protected:
    static Polygon<double> unitSquareAt(double x, double y, double size = 1.0) {
        return Polygon<double>(Ring<double>(std::vector<Vec2DReal>{
            {x,y},{x,y+size},{x+size,y+size},{x+size,y}
        }));
    }
    // two disjoint 2x2 squares: [0,2]x[0,2] and [5,7]x[0,2]
    static Polygon<double> left()  { return unitSquareAt(0,0,2); }
    static Polygon<double> right() { return unitSquareAt(5,0,2); }

    static MultiPolygon<Polygon<double>> native() {
        return MultiPolygon<Polygon<double>>(std::vector<Polygon<double>>{left(), right()});
    }

    void SetUp() override {
        multi = std::make_unique<OGRMultiPolygonAdapter>(std::vector<Polygon<double>>{left(), right()});
    }

    std::unique_ptr<OGRMultiPolygonAdapter> multi;
};

// ============================================================================
// Construction
// ============================================================================

TEST_F(OGRMultiPolygonAdapterTest, initFromPolygonRange) {
    OGRMultiPolygonAdapter adapted(std::vector<Polygon<double>>{left(), right()});
    EXPECT_EQ(adapted.size(), 2u);
    EXPECT_DOUBLE_EQ(adapted.area(), 8.0);
}

TEST_F(OGRMultiPolygonAdapterTest, initFromSinglePolygon) {
    OGRPolygonAdapter polygon(left());
    OGRMultiPolygonAdapter adapted(std::vector<OGRPolygonAdapter>{polygon});
    EXPECT_EQ(adapted.size(), 1u);
    EXPECT_DOUBLE_EQ(adapted.area(), 4.0);
}

TEST_F(OGRMultiPolygonAdapterTest, initFromMultiPolygon) {
    OGRMultiPolygonAdapter adapted(native());
    EXPECT_EQ(adapted.size(), 2u);
    EXPECT_DOUBLE_EQ(adapted.area(), native().area());
}

TEST_F(OGRMultiPolygonAdapterTest, initFromOGRPtr) {
    auto * ogrMulti = static_cast<OGRMultiPolygon*>(OGRGeometryFactory::createGeometry(wkbMultiPolygon));
    OGRPolygonAdapter leftPolygon(left());
    OGRPolygonAdapter rightPolygon(right());
    ogrMulti->addGeometry(leftPolygon.raw());
    ogrMulti->addGeometry(rightPolygon.raw());
    auto ptr = OGRUniquePtr<OGRMultiPolygon>(ogrMulti);
    OGRMultiPolygonAdapter adapted(std::move(ptr));
    EXPECT_EQ(adapted.size(), 2u);
    EXPECT_DOUBLE_EQ(adapted.area(), 8.0);
}

TEST_F(OGRMultiPolygonAdapterTest, initSkipsInvalidPolygons) {
    // the second polygon is a duplicate of the first and must be skipped
    OGRMultiPolygonAdapter adapted(std::vector<Polygon<double>>{left(), left(), right()});
    EXPECT_EQ(adapted.size(), 2u);
}

TEST_F(OGRMultiPolygonAdapterTest, copyAndMove) {
    OGRMultiPolygonAdapter copy(*multi);
    EXPECT_EQ(copy, *multi);
    OGRMultiPolygonAdapter moved(std::move(copy));
    EXPECT_EQ(moved, *multi);
}

// ============================================================================
// Polygon collection
// ============================================================================

TEST_F(OGRMultiPolygonAdapterTest, getPolygons) {
    EXPECT_SIZE(multi->getPolygons(), 2);
    EXPECT_CONTAINS(multi->getPolygons(), OGRPolygonAdapter(left()));
    EXPECT_CONTAINS(multi->getPolygons(), OGRPolygonAdapter(right()));
}

TEST_F(OGRMultiPolygonAdapterTest, addPolygon) {
    OGRPolygonAdapter further(unitSquareAt(10,10,2));
    EXPECT_TRUE(multi->addPolygon(further));
    EXPECT_EQ(multi->size(), 3u);
    EXPECT_DOUBLE_EQ(multi->area(), 12.0);
}

TEST_F(OGRMultiPolygonAdapterTest, addPolygonRejectsDuplicate) {
    OGRPolygonAdapter duplicate(left());
    EXPECT_FALSE(multi->addPolygon(duplicate));
    EXPECT_EQ(multi->size(), 2u);
}

TEST_F(OGRMultiPolygonAdapterTest, addPolygonRejectsCrossingPolygon) {
    OGRPolygonAdapter crossing(unitSquareAt(1,1,2)); // overlaps the left square
    EXPECT_FALSE(multi->addPolygon(crossing));
    EXPECT_EQ(multi->size(), 2u);
}

TEST_F(OGRMultiPolygonAdapterTest, addPolygonRejectsContainedPolygon) {
    OGRPolygonAdapter contained(unitSquareAt(0.5,0.5,0.5));
    EXPECT_FALSE(multi->addPolygon(contained));
    EXPECT_EQ(multi->size(), 2u);
}

TEST_F(OGRMultiPolygonAdapterTest, removePolygon) {
    OGRPolygonAdapter leftPolygon(left());
    EXPECT_TRUE(multi->removePolygon(leftPolygon));
    EXPECT_EQ(multi->size(), 1u);
    EXPECT_DOUBLE_EQ(multi->area(), 4.0);
    EXPECT_NOT_CONTAINS(multi->getPolygons(), OGRPolygonAdapter(left()));
}

TEST_F(OGRMultiPolygonAdapterTest, removePolygonWhichIsNotPresent) {
    OGRPolygonAdapter absent(unitSquareAt(100,100,2));
    EXPECT_FALSE(multi->removePolygon(absent));
    EXPECT_EQ(multi->size(), 2u);
}

// ============================================================================
// Geometry properties
// ============================================================================

TEST_F(OGRMultiPolygonAdapterTest, area) {
    EXPECT_DOUBLE_EQ(multi->area(), 8.0);
    EXPECT_DOUBLE_EQ(multi->area(), native().area());
}

TEST_F(OGRMultiPolygonAdapterTest, centroid) {
    // both squares have the same area, so the centroid is the midpoint of their centroids
    EXPECT_EQ(multi->centroid(), Vec2DReal(3.5,1));
    EXPECT_EQ(multi->centroid(), native().centroid());
}

TEST_F(OGRMultiPolygonAdapterTest, centroidIsAreaWeighted) {
    OGRMultiPolygonAdapter weighted(std::vector<Polygon<double>>{unitSquareAt(0,0,1), unitSquareAt(10,0,3)});
    MultiPolygon<Polygon<double>> weightedReference(std::vector<Polygon<double>>{unitSquareAt(0,0,1), unitSquareAt(10,0,3)});
    EXPECT_EQ(weighted.centroid(), weightedReference.centroid());
}

TEST_F(OGRMultiPolygonAdapterTest, aaBB) {
    auto bbox = multi->aaBB();
    EXPECT_DOUBLE_EQ(bbox.left(), 0.0);
    EXPECT_DOUBLE_EQ(bbox.right(), 7.0);
    EXPECT_DOUBLE_EQ(bbox.top(), 2.0);
    EXPECT_DOUBLE_EQ(bbox.bottom(), 0.0);
}

// ============================================================================
// Point containment
// ============================================================================

TEST_F(OGRMultiPolygonAdapterTest, isInside) {
    EXPECT_TRUE(multi->isInside(Vec2DReal(1,1)));    // in the left square
    EXPECT_TRUE(multi->isInside(Vec2DReal(6,1)));    // in the right square
    EXPECT_FALSE(multi->isInside(Vec2DReal(3,1)));   // in the gap between them
    EXPECT_FALSE(multi->isInside(Vec2DReal(0,0)));   // on the boundary
}

TEST_F(OGRMultiPolygonAdapterTest, isOnBoundary) {
    EXPECT_TRUE(multi->isOnBoundary(Vec2DReal(0,0)));
    EXPECT_TRUE(multi->isOnBoundary(Vec2DReal(5,1)));
    EXPECT_FALSE(multi->isOnBoundary(Vec2DReal(1,1)));
    EXPECT_FALSE(multi->isOnBoundary(Vec2DReal(3,1)));
}

TEST_F(OGRMultiPolygonAdapterTest, isOutside) {
    EXPECT_TRUE(multi->isOutside(Vec2DReal(3,1)));   // gap
    EXPECT_TRUE(multi->isOutside(Vec2DReal(-1,-1)));
    EXPECT_FALSE(multi->isOutside(Vec2DReal(1,1)));
    EXPECT_FALSE(multi->isOutside(Vec2DReal(0,0)));
}

TEST_F(OGRMultiPolygonAdapterTest, pointLocationMatchesMultiPolygon) {
    auto ref = native();
    std::vector<Vec2DReal> probes {{1,1},{6,1},{3,1},{0,0},{5,1},{-1,-1},{2,2}};
    for(const auto & probe : probes){
        EXPECT_EQ(multi->isInside(probe), ref.isInside(probe)) << probe.toString();
        EXPECT_EQ(multi->isOnBoundary(probe), ref.isOnBoundary(probe)) << probe.toString();
        EXPECT_EQ(multi->isOutside(probe), ref.isOutside(probe)) << probe.toString();
        EXPECT_EQ(multi->contains(probe), ref.contains(probe)) << probe.toString();
    }
}

TEST_F(OGRMultiPolygonAdapterTest, containsPoint) {
    EXPECT_TRUE(multi->contains(Vec2DReal(1,1)));
    EXPECT_TRUE(multi->contains(Vec2DReal(0,0)));   // boundary counts
    EXPECT_FALSE(multi->contains(Vec2DReal(3,1)));  // gap
}

// ============================================================================
// Segment / polygon containment
// ============================================================================

TEST_F(OGRMultiPolygonAdapterTest, containsSegment) {
    EXPECT_TRUE(multi->contains(Segment(Vec2DReal(0.5,0.5), Vec2DReal(1.5,1.5))));
    EXPECT_TRUE(multi->contains(Segment(Vec2DReal(5.5,0.5), Vec2DReal(6.5,1.5))));
    // a segment spanning both squares is not contained, the gap is not part of the multi-polygon
    EXPECT_FALSE(multi->contains(Segment(Vec2DReal(1,1), Vec2DReal(6,1))));
    EXPECT_FALSE(multi->contains(Segment(Vec2DReal(20,20), Vec2DReal(21,21))));
}

TEST_F(OGRMultiPolygonAdapterTest, containsPolygon) {
    Polygon<double> inLeft(unitSquareAt(0.5,0.5,0.5));
    EXPECT_TRUE(multi->contains(inLeft));
    EXPECT_EQ(multi->contains(inLeft), native().contains(inLeft));

    Polygon<double> outside(unitSquareAt(20,20,1));
    EXPECT_FALSE(multi->contains(outside));
    EXPECT_EQ(multi->contains(outside), native().contains(outside));
}

// ============================================================================
// Linear geometry intersection
// ============================================================================

TEST_F(OGRMultiPolygonAdapterTest, intersects) {
    // a horizontal line at y=1 crosses both squares
    EXPECT_TRUE(multi->intersects(Line<double>::horizontalLine(1)));
    EXPECT_EQ(multi->intersects(Line<double>::horizontalLine(1)), native().intersects(Line<double>::horizontalLine(1)));
    // a segment strictly inside one square crosses nothing
    EXPECT_FALSE(multi->intersects(Segment(Vec2DReal(0.5,0.5), Vec2DReal(1.5,1.5))));
    EXPECT_FALSE(multi->intersects(Segment(Vec2DReal(20,20), Vec2DReal(21,21))));
}

TEST_F(OGRMultiPolygonAdapterTest, intersections) {
    // a horizontal line at y=1 meets each of the two squares twice
    auto result = multi->intersections(Line<double>::horizontalLine(1));
    EXPECT_SIZE(result, 4);
    EXPECT_CONTAINS(result, Vec2DReal(0,1));
    EXPECT_CONTAINS(result, Vec2DReal(2,1));
    EXPECT_CONTAINS(result, Vec2DReal(5,1));
    EXPECT_CONTAINS(result, Vec2DReal(7,1));

    EXPECT_EMPTY(multi->intersections(Segment(Vec2DReal(20,20), Vec2DReal(21,21))));
}

// ============================================================================
// Spatial relations
// ============================================================================

TEST_F(OGRMultiPolygonAdapterTest, crosses) {
    Polygon<double> overlappingLeft(unitSquareAt(1,1,2));
    EXPECT_TRUE(multi->crosses(overlappingLeft));
    EXPECT_EQ(multi->crosses(overlappingLeft), native().crosses(overlappingLeft));

    Polygon<double> disjoint(unitSquareAt(20,20,1));
    EXPECT_FALSE(multi->crosses(disjoint));
    EXPECT_EQ(multi->crosses(disjoint), native().crosses(disjoint));

    // a polygon in the gap touches nothing and crosses nothing
    Polygon<double> inGap(Polygon<double>(Ring<double>(std::vector<Vec2DReal>{{3,0.5},{3,1.5},{4,1.5},{4,0.5}})));
    EXPECT_FALSE(multi->crosses(inGap));
}

TEST_F(OGRMultiPolygonAdapterTest, touches) {
    Polygon<double> neighbour(Polygon<double>(Ring<double>(std::vector<Vec2DReal>{{2,0},{2,2},{3,2},{3,0}})));
    EXPECT_TRUE(multi->touches(neighbour));
    EXPECT_EQ(multi->touches(neighbour), native().touches(neighbour));

    Polygon<double> disjoint(unitSquareAt(20,20,1));
    EXPECT_FALSE(multi->touches(disjoint));
    EXPECT_EQ(multi->touches(disjoint), native().touches(disjoint));
}

TEST_F(OGRMultiPolygonAdapterTest, distanceToPolygon) {
    // the closest of the two squares determines the distance
    Polygon<double> away(Polygon<double>(Ring<double>(std::vector<Vec2DReal>{{9,0},{9,2},{10,2},{10,0}})));
    EXPECT_DOUBLE_EQ(multi->distance(away), 2.0);
    EXPECT_DOUBLE_EQ(multi->distance(away), native().distance(away));

    // a touching polygon reports 0
    Polygon<double> neighbour(Polygon<double>(Ring<double>(std::vector<Vec2DReal>{{2,0},{2,2},{3,2},{3,0}})));
    EXPECT_DOUBLE_EQ(multi->distance(neighbour), 0.0);

    // a contained polygon has no gap to the multi-polygon
    Polygon<double> inside(unitSquareAt(0.5,0.5,0.5));
    EXPECT_DOUBLE_EQ(multi->distance(inside), 0.0); // contained, hence no gap
}

TEST_F(OGRMultiPolygonAdapterTest, distanceToMultiPolygon) {
    OGRMultiPolygonAdapter other(std::vector<Polygon<double>>{unitSquareAt(9,0,1), unitSquareAt(20,20,1)});
    EXPECT_DOUBLE_EQ(multi->distance(other), 2.0);
}

// Cross-checks the OGRMultiPolygonAdapter-to-OGRMultiPolygonAdapter fast path (native GEOS
// Distance()) against the generic IMultiPolygon template path, which distanceToMultiPolygon above
// does not exercise since its "other" happens to only ever be the closest member.
TEST_F(OGRMultiPolygonAdapterTest, distanceAgreesBetweenNativeAndGenericPath) {
    auto check = [](const OGRMultiPolygonAdapter & lhs, const MultiPolygon<Polygon<double>> & rhsNative) {
        OGRMultiPolygonAdapter rhsAdapter(rhsNative);
        EXPECT_DOUBLE_EQ(lhs.distance(rhsAdapter), lhs.distance(rhsNative))
            << "lhs=" << lhs.toString() << " rhs=" << rhsAdapter.toString();
    };

    // disjoint: nearest pair is left() to the close square
    check(*multi, MultiPolygon<Polygon<double>>(std::vector<Polygon<double>>{unitSquareAt(9,0,1), unitSquareAt(20,20,1)}));

    // touching: one member of other touches left()
    check(*multi, MultiPolygon<Polygon<double>>(std::vector<Polygon<double>>{unitSquareAt(2,0,1), unitSquareAt(20,20,1)}));

    // contained: one member of other sits fully inside right()
    check(*multi, MultiPolygon<Polygon<double>>(std::vector<Polygon<double>>{unitSquareAt(5.5,0.5,0.5), unitSquareAt(20,20,1)}));

    // through a hole: one member of other sits inside a hole of a polygon with holes
    Ring<double> outer(std::vector<Vec2DReal>{{0,0},{0,10},{10,10},{10,0}});
    Ring<double> hole(std::vector<Vec2DReal>{{4,4},{4,6},{6,6},{6,4}});
    Polygon<double> withHole(outer, std::vector<Ring<double>>{hole});
    OGRMultiPolygonAdapter holed(std::vector<Polygon<double>>{withHole});
    check(holed, MultiPolygon<Polygon<double>>(std::vector<Polygon<double>>{
        Polygon<double>(Ring<double>(std::vector<Vec2DReal>{{4.5,4.5},{4.5,5.5},{5.5,5.5},{5.5,4.5}}))
    }));

    // identical multi-polygon
    check(*multi, native());
}

TEST_F(OGRMultiPolygonAdapterTest, containsInHole) {
    Ring<double> outer(std::vector<Vec2DReal>{{0,0},{0,10},{10,10},{10,0}});
    Ring<double> hole(std::vector<Vec2DReal>{{4,4},{4,6},{6,6},{6,4}});
    Polygon<double> withHole(outer, std::vector<Ring<double>>{hole});
    OGRMultiPolygonAdapter holed(std::vector<Polygon<double>>{withHole});

    Polygon<double> inHole(Polygon<double>(Ring<double>(std::vector<Vec2DReal>{{4.5,4.5},{4.5,5.5},{5.5,5.5},{5.5,4.5}})));
    EXPECT_TRUE(holed.containsInHole(inHole));
    EXPECT_FALSE(multi->containsInHole(inHole));
}

// ============================================================================
// Base class behavior
// ============================================================================

TEST_F(OGRMultiPolygonAdapterTest, equality) {
    EXPECT_EQ(*multi, *multi);
    EXPECT_EQ(*multi, OGRMultiPolygonAdapter(std::vector<Polygon<double>>{left(), right()}));
    // order of the polygons does not matter
    EXPECT_EQ(*multi, OGRMultiPolygonAdapter(std::vector<Polygon<double>>{right(), left()}));
    EXPECT_NE(*multi, OGRMultiPolygonAdapter(std::vector<Polygon<double>>{left()}));
}

TEST_F(OGRMultiPolygonAdapterTest, hashIsIndependentOfPolygonOrder) {
    OGRMultiPolygonAdapter reversed(std::vector<Polygon<double>>{right(), left()});
    EXPECT_EQ(multi->hash(), reversed.hash());
}

TEST_F(OGRMultiPolygonAdapterTest, toString) {
    auto str = multi->toString();
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str.find("MULTIPOLYGON"), std::string::npos);
}
