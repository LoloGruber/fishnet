#include <gtest/gtest.h>
#include <fishnet/TestUtil.hpp>
#include <fishnet/OGRGeometryWrapper.hpp>
#include <fishnet/Ring.hpp>
#include <fishnet/Polygon.hpp>

using namespace fishnet::geometry;
using namespace fishnet::test;

class OGRGeometryAdapterTest : public ::testing::Test {
protected:
    static Ring<double> squareRing() {
        return Ring<double>(std::vector<Vec2DReal>{{0,0},{0,2},{2,2},{2,0}});
    }
    static Polygon<double> squarePolygon() {
        return Polygon<double>(squareRing());
    }

    static OGRUniquePtr<OGRGeometry> asGeometry(const OGRGeometry & geometry) {
        return OGRUniquePtr<OGRGeometry>(geometry.clone());
    }

    static OGRUniquePtr<OGRGeometry> ringGeometry(bool closed = true) {
        auto * ring = static_cast<OGRLinearRing*>(OGRGeometryFactory::createGeometry(wkbLinearRing));
        ring->addPoint(0,0);
        ring->addPoint(0,2);
        ring->addPoint(2,2);
        ring->addPoint(2,0);
        if(closed)
            ring->closeRings();
        return OGRUniquePtr<OGRGeometry>(ring);
    }

    static OGRUniquePtr<OGRGeometry> polygonGeometry() {
        OGRPolygonAdapter polygon {squarePolygon()};
        return asGeometry(*polygon.raw());
    }

    static OGRUniquePtr<OGRGeometry> multiPolygonGeometry() {
        OGRMultiPolygonAdapter multiPolygon {std::vector<Polygon<double>>{squarePolygon()}};
        return asGeometry(*multiPolygon.raw());
    }

    static OGRUniquePtr<OGRGeometry> pointGeometry() {
        return OGRUniquePtr<OGRGeometry>(new OGRPoint(1,1));
    }

    static OGRUniquePtr<OGRGeometry> lineStringGeometry() {
        auto * line = static_cast<OGRLineString*>(OGRGeometryFactory::createGeometry(wkbLineString));
        line->addPoint(0,0);
        line->addPoint(1,1);
        return OGRUniquePtr<OGRGeometry>(line);
    }
};

// ============================================================================
// Type inspection
// ============================================================================

TEST_F(OGRGeometryAdapterTest, wkbType) {
    EXPECT_EQ(OGRGeometryAdapter(polygonGeometry()).wkbType(), wkbPolygon);
    EXPECT_EQ(OGRGeometryAdapter(multiPolygonGeometry()).wkbType(), wkbMultiPolygon);
    EXPECT_EQ(OGRGeometryAdapter(pointGeometry()).wkbType(), wkbPoint);
    // OGR has no WKB type of its own for a linear ring and reports it as a line string
    EXPECT_EQ(OGRGeometryAdapter(ringGeometry()).wkbType(), wkbLineString);
}

TEST_F(OGRGeometryAdapterTest, geometryName) {
    EXPECT_EQ(OGRGeometryAdapter(polygonGeometry()).geometryName(), "POLYGON");
    EXPECT_EQ(OGRGeometryAdapter(multiPolygonGeometry()).geometryName(), "MULTIPOLYGON");
    EXPECT_EQ(OGRGeometryAdapter(ringGeometry()).geometryName(), "LINEARRING");
}

TEST_F(OGRGeometryAdapterTest, typePredicates) {
    OGRGeometryAdapter polygon {polygonGeometry()};
    EXPECT_TRUE(polygon.isPolygon());
    EXPECT_FALSE(polygon.isRing());
    EXPECT_FALSE(polygon.isMultiPolygon());

    OGRGeometryAdapter multiPolygon {multiPolygonGeometry()};
    EXPECT_TRUE(multiPolygon.isMultiPolygon());
    EXPECT_FALSE(multiPolygon.isPolygon());
    EXPECT_FALSE(multiPolygon.isRing());

    OGRGeometryAdapter ring {ringGeometry()};
    EXPECT_TRUE(ring.isRing());
    EXPECT_FALSE(ring.isPolygon());
    EXPECT_FALSE(ring.isMultiPolygon());

    OGRGeometryAdapter point {pointGeometry()};
    EXPECT_FALSE(point.isRing());
    EXPECT_FALSE(point.isPolygon());
    EXPECT_FALSE(point.isMultiPolygon());
}

TEST_F(OGRGeometryAdapterTest, isEmpty) {
    EXPECT_FALSE(OGRGeometryAdapter(polygonGeometry()).isEmpty());
    auto * emptyPolygon = static_cast<OGRPolygon*>(OGRGeometryFactory::createGeometry(wkbPolygon));
    EXPECT_TRUE(OGRGeometryAdapter(OGRUniquePtr<OGRGeometry>(emptyPolygon)).isEmpty());
}

// ============================================================================
// Conversions
// ============================================================================

TEST_F(OGRGeometryAdapterTest, toPolygon) {
    OGRGeometryAdapter geometry {polygonGeometry()};
    auto polygon = geometry.toPolygon();
    ASSERT_VALUE(polygon);
    EXPECT_DOUBLE_EQ(polygon.value().area(), 4.0);
    EXPECT_EQ(polygon.value(), OGRPolygonAdapter(squarePolygon()));
}

TEST_F(OGRGeometryAdapterTest, toMultiPolygon) {
    OGRGeometryAdapter geometry {multiPolygonGeometry()};
    auto multiPolygon = geometry.toMultiPolygon();
    ASSERT_VALUE(multiPolygon);
    EXPECT_EQ(multiPolygon.value().size(), 1u);
    EXPECT_DOUBLE_EQ(multiPolygon.value().area(), 4.0);
}

TEST_F(OGRGeometryAdapterTest, toRing) {
    OGRGeometryAdapter geometry {ringGeometry()};
    auto ring = geometry.toRing();
    ASSERT_VALUE(ring);
    EXPECT_DOUBLE_EQ(ring.value().area(), 4.0);
    EXPECT_EQ(ring.value(), OGRRingAdapter(squareRing()));
    EXPECT_SORTED_RANGE_EQ(ring.value().getPoints(), squareRing().getPoints());
}

TEST_F(OGRGeometryAdapterTest, toRingClosesAnUnclosedRing) {
    OGRGeometryAdapter geometry {ringGeometry(false)};
    auto ring = geometry.toRing();
    ASSERT_VALUE(ring);
    EXPECT_SIZE(ring.value().getPoints(), 4);
    EXPECT_DOUBLE_EQ(ring.value().area(), 4.0);
}

TEST_F(OGRGeometryAdapterTest, toRingAcceptsAPlainLineString) {
    // a closed line string describes a ring just as well, and must not be downcast to OGRLinearRing
    auto * line = static_cast<OGRLineString*>(OGRGeometryFactory::createGeometry(wkbLineString));
    line->addPoint(0,0);
    line->addPoint(0,2);
    line->addPoint(2,2);
    line->addPoint(2,0);
    line->addPoint(0,0);
    OGRGeometryAdapter geometry {OGRUniquePtr<OGRGeometry>(line)};
    auto ring = geometry.toRing();
    ASSERT_VALUE(ring);
    EXPECT_DOUBLE_EQ(ring.value().area(), 4.0);
}

// ============================================================================
// Conversions of the wrong type yield an empty Option
// ============================================================================

TEST_F(OGRGeometryAdapterTest, toPolygonOfAnotherTypeIsEmpty) {
    EXPECT_EMPTY(OGRGeometryAdapter(multiPolygonGeometry()).toPolygon());
    EXPECT_EMPTY(OGRGeometryAdapter(ringGeometry()).toPolygon());
    EXPECT_EMPTY(OGRGeometryAdapter(pointGeometry()).toPolygon());
}

TEST_F(OGRGeometryAdapterTest, toMultiPolygonOfAnotherTypeIsEmpty) {
    EXPECT_EMPTY(OGRGeometryAdapter(polygonGeometry()).toMultiPolygon());
    EXPECT_EMPTY(OGRGeometryAdapter(ringGeometry()).toMultiPolygon());
    EXPECT_EMPTY(OGRGeometryAdapter(pointGeometry()).toMultiPolygon());
}

TEST_F(OGRGeometryAdapterTest, toRingOfAnotherTypeIsEmpty) {
    EXPECT_EMPTY(OGRGeometryAdapter(polygonGeometry()).toRing());
    EXPECT_EMPTY(OGRGeometryAdapter(multiPolygonGeometry()).toRing());
    EXPECT_EMPTY(OGRGeometryAdapter(pointGeometry()).toRing());
    // a line string with too few points cannot enclose an area
    EXPECT_EMPTY(OGRGeometryAdapter(lineStringGeometry()).toRing());
}

TEST_F(OGRGeometryAdapterTest, conversionOfAnEmptyGeometryIsEmpty) {
    // an empty geometry would yield an adapter without a boundary to work with
    auto * emptyPolygon = static_cast<OGRPolygon*>(OGRGeometryFactory::createGeometry(wkbPolygon));
    EXPECT_EMPTY(OGRGeometryAdapter(OGRUniquePtr<OGRGeometry>(emptyPolygon)).toPolygon());

    auto * emptyMulti = static_cast<OGRMultiPolygon*>(OGRGeometryFactory::createGeometry(wkbMultiPolygon));
    EXPECT_EMPTY(OGRGeometryAdapter(OGRUniquePtr<OGRGeometry>(emptyMulti)).toMultiPolygon());

    auto * emptyRing = static_cast<OGRLinearRing*>(OGRGeometryFactory::createGeometry(wkbLinearRing));
    EXPECT_EMPTY(OGRGeometryAdapter(OGRUniquePtr<OGRGeometry>(emptyRing)).toRing());
}

// ============================================================================
// Behaviour of the wrapper itself
// ============================================================================

TEST_F(OGRGeometryAdapterTest, conversionLeavesTheWrappedGeometryIntact) {
    OGRGeometryAdapter geometry {polygonGeometry()};
    auto first = geometry.toPolygon();
    auto second = geometry.toPolygon();
    ASSERT_VALUE(first);
    ASSERT_VALUE(second);
    EXPECT_EQ(first.value(), second.value());
    EXPECT_TRUE(geometry.isPolygon());
}

TEST_F(OGRGeometryAdapterTest, initFromConcreteAdapters) {
    OGRPolygonAdapter polygon {squarePolygon()};
    EXPECT_TRUE(OGRGeometryAdapter(polygon).isPolygon());

    OGRRingAdapter ring {squareRing()};
    EXPECT_TRUE(OGRGeometryAdapter(ring).isRing());

    OGRMultiPolygonAdapter multiPolygon {std::vector<OGRPolygonAdapter>{polygon}};
    EXPECT_TRUE(OGRGeometryAdapter(multiPolygon).isMultiPolygon());
}

TEST_F(OGRGeometryAdapterTest, roundTripThroughTheWrapper) {
    OGRPolygonAdapter polygon {squarePolygon()};
    OGRGeometryAdapter wrapped {polygon};
    auto unwrapped = wrapped.toPolygon();
    ASSERT_VALUE(unwrapped);
    EXPECT_EQ(unwrapped.value(), polygon);
}

TEST_F(OGRGeometryAdapterTest, hashAndToString) {
    OGRGeometryAdapter polygon {polygonGeometry()};
    OGRGeometryAdapter samePolygon {polygonGeometry()};
    EXPECT_EQ(polygon.hash(), samePolygon.hash());
    EXPECT_NE(polygon.hash(), OGRGeometryAdapter(pointGeometry()).hash());

    EXPECT_NE(polygon.toString().find("POLYGON"), std::string::npos);
    // a wrapped linear ring must hash as well, although it has no WKB representation
    EXPECT_NO_FATAL_FAILURE(OGRGeometryAdapter(ringGeometry()).hash());
}

TEST_F(OGRGeometryAdapterTest, chainingWithOption) {
    OGRGeometryAdapter geometry {polygonGeometry()};
    auto area = geometry.toPolygon().transform([](const auto & polygon){return polygon.area();});
    ASSERT_VALUE(area);
    EXPECT_DOUBLE_EQ(area.value(), 4.0);

    auto noArea = OGRGeometryAdapter(pointGeometry()).toPolygon().transform([](const auto & polygon){return polygon.area();});
    EXPECT_EMPTY(noArea);
}
