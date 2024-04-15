#include <gtest/gtest.h>
#include <fishnet/SweepLine.hpp>
#include <fishnet/Vec2D.hpp>
#include <fishnet/PolygonFilter.hpp>
#include "ShapeSamples.h"
#include "Testutil.h"

using namespace testutil;
using namespace fishnet::geometry;

struct xOrdering{
    bool operator()(const Vec2D<double> & lhs, const Vec2D<double> & rhs) const noexcept {
        return lhs.x < rhs.x;
    }
};

struct yOrdering {
    bool operator()(const Vec2D<double> & lhs, const Vec2D<double> & rhs) const noexcept {
        return lhs.y < rhs.y;
    }
};

using MySweepLine = SweepLine<Vec2D<double>,std::string, xOrdering>;

struct MyPointInsertEvent: public MySweepLine::InsertEvent{
public:
    MyPointInsertEvent(const Vec2D<double> & obj):MySweepLine::InsertEvent(obj){}
    
    virtual void process(MySweepLine & sweepLine, std::vector<std::string> & output) const {
        sweepLine.addSLS(this->obj);
        output.push_back("Inserting: "+obj->toString());
    }

    virtual double eventPoint() const noexcept {
        return this->getObject().y;
    }
};

struct MyPointRemoveEvent: public MySweepLine::RemoveEvent{
public:
    MyPointRemoveEvent(const Vec2D<double> & obj):MySweepLine::RemoveEvent(obj){}
    
    virtual void process(MySweepLine & sweepLine, std::vector<std::string> & output) const {
        sweepLine.removeSLS(this->obj);
        output.push_back("Removing: "+obj->toString());
    }

    virtual double eventPoint() const noexcept {
        return this->getObject().y;
    }
};


TEST(SweepLineTest, simple) {
    std::vector<Vec2D<double>>  points  =  {
        {1,1},{2,1},{2,5},{5,1},{-2,4}
    };
    auto eventMapper = [](const Vec2D<double> & vec){
        std::vector<MySweepLine::eventPointer> events;
        events.emplace_back(std::make_unique<MyPointInsertEvent>(vec));
        events.emplace_back(std::make_unique<MyPointRemoveEvent>(vec));
        return events;
    };
    std::vector<std::string> output;
    MySweepLine sweepLine;
    sweepLine.addEvents(points,eventMapper);

    auto result = sweepLine.sweep(output);
    EXPECT_SIZE(result,10);
}

TEST(SweepLineTest, polygonFiltering) {
    std::vector<SimplePolygon<double>> polygons =  {
        SimplePolygonSamples::aaBB({4,4},{0,0}),
        SimplePolygonSamples::aaBB({2,2},{3,3}),
        SimplePolygonSamples::aaBB({-1.1,-1.1},{-1.0,-1.0})
    };


    auto binaryFilterCondition = [](const SimplePolygon<double> & p, const SimplePolygon<double> & underTest){
        return not p.contains(underTest);
    };
    auto areaFilter = [](const SimplePolygon<double> & p){
        return p.area() >= 0.5;
    };
    auto filtered = filter(polygons,binaryFilterCondition,areaFilter);

    EXPECT_CONTAINS(filtered,polygons[0]);
    EXPECT_SIZE(filtered,1);

    auto filtered_view = filter(std::views::all(polygons) | std::views::transform([](const auto & v){return v;}),binaryFilterCondition,areaFilter);
    EXPECT_SIZE(filtered_view, 1); //added test for views, to discover reference errors

}