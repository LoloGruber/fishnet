#pragma once
#include <fishnet/IGeometry.hpp>
#include <fishnet/Segment.hpp>
#include <fishnet/Rectangle.hpp>
#include "OGRAdapterBase.hpp"

namespace fishnet::geometry{

class OGRRingAdapter: public OGRAdapterBase<OGRLinearRing>{
private:
    using Base = OGRAdapterBase<OGRLinearRing>;
public:
    using numeric_type = double;
    static constexpr GeometryType type = GeometryType::RING;

    OGRRingAdapter(OGRUniquePtr<OGRLinearRing> && ringPtr):Base(std::move(ringPtr)) {}
    OGRRingAdapter(IRing auto const & ring):Base(OGRUniquePtr<OGRLinearRing>(static_cast<OGRLinearRing*>(OGRGeometryFactory::createGeometry(wkbLinearRing)))){
        for(const auto & point : ring.getPoints()){
            geomPtr->addPoint(point.getX(), point.getY());
        }
    }

    auto getSegments() const -> fishnet::util::forward_range_of<fishnet::geometry::Segment<double>> auto {
        return std::ranges::views::iota(0, geomPtr->getNumPoints() - 1)
            | std::ranges::views::transform([this](int i) {
                Vec2DReal p1 {geomPtr->getX(i), geomPtr->getY(i)};
                Vec2DReal p2 {geomPtr->getX(i + 1), geomPtr->getY(i + 1)};
                return Segment<double>(p1, p2);
            });
    }

    auto getPoints() const -> fishnet::util::forward_range_of<fishnet::geometry::Vec2DReal> auto {
        return std::ranges::views::iota(0, geomPtr->getNumPoints())
            | std::ranges::views::transform([this](int i) {
                return Vec2DReal(geomPtr->getX(i), geomPtr->getY(i));
            });
    }

    const OGRRingAdapter & getBoundary() const{
        return *this;
    }

    std::vector<OGRRingAdapter> getHoles() const{
        return {};
    }

    double area() const{
        return geomPtr->get_Area();
    }

    Vec2DReal centroid() const {
        OGRPoint point;
        geomPtr->Centroid(&point);
        return Vec2DReal(point.getX(), point.getY());
    }

    Rectangle<double> aaBB() const{
        OGREnvelope boundingBox;
        geomPtr->getEnvelope(&boundingBox);
        return Rectangle<double>(boundingBox.MinX, boundingBox.MaxY, boundingBox.MaxX, boundingBox.MinY);
    }

    bool isInside(const IPoint auto & point) const{
        OGRPoint ogrPoint(point.getX(), point.getY());
        return geomPtr->Contains(&ogrPoint) && !geomPtr->Touches(&ogrPoint);
    }

    bool isOnBoundary(const IPoint auto & point) const{
        OGRPoint ogrPoint(point.getX(), point.getY());
        return geomPtr->Touches(&ogrPoint);
    }

    bool isOutside(const IPoint auto & point) const{
        OGRPoint ogrPoint(point.getX(), point.getY());
        return geomPtr->Disjoint(&ogrPoint);
    }

    bool contains(const IPoint auto & point) const{
        OGRPoint ogrPoint(point.getX(), point.getY());
        return geomPtr->Contains(&ogrPoint);
    }

    bool contains(const ISegment auto & segment) const{
        OGRLineString ogrLine;
        ogrLine.addPoint(segment.p().getX(), segment.p().getY());
        ogrLine.addPoint(segment.q().getX(), segment.q().getY());
        return geomPtr->Contains(&ogrLine);
    }

    bool contains(const IRing auto & ring) const{
        OGRLinearRing ogrRing;
        for (const auto & point : ring.getPoints()) {
            ogrRing.addPoint(point.getX(), point.getY());
        }
        return geomPtr->Contains(&ogrRing);
    }

    bool contains(const OGRRingAdapter & other) const{
        return geomPtr->Contains(other.geomPtr.get());
    }

    bool intersects(const LinearGeometry auto & linearFeature) const{
        return this->intersections(linearFeature).size() > 0;
    }

    std::unordered_set<Vec2DReal> intersections(const LinearGeometry auto & linearFeature) const{
        auto intersectionRange = this->getSegments()
            | std::views::transform([linearFeature](const auto & segment){return segment.intersection(linearFeature);})
            | std::views::filter([](const auto & optIntersection){return optIntersection.has_value();})
            | std::views::transform([](const auto & optIntersection){return optIntersection.value();});
        return std::unordered_set<Vec2D<double>> {std::ranges::begin(intersectionRange), std::ranges::end(intersectionRange)};
    }

    bool crosses(const IRing auto & other) const{
        return std::ranges::any_of(this->getSegments(),[&other](const auto & s){return other.intersects(s);})
            || std::ranges::any_of(other.getSegments(),[this](const auto & s){return this->intersects(s);});
    }

    bool crosses(const OGRRingAdapter & other) const{
        return geomPtr->Crosses(other.geomPtr.get());
    }

    bool touches(const IRing auto & other) const{
        if(this->crosses(other)) return false;
        if(this->contains(other) || other.contains(*this)) return false;
        for(const auto & p : other.getPoints()){
            if(this->isOnBoundary(p)) return true;
        }
        return false;
    }

    bool touches(const OGRRingAdapter & other) const{
        return geomPtr->Touches(other.geomPtr.get());
    }

    double distance(const IRing auto & other) const{
        if(this->contains(other) or other.contains(*this) or this->crosses(other))
             return -1;
        return shapeDistance(*this,other);
    }

    double distance(const OGRRingAdapter & other) const{
        return geomPtr->Distance(other.geomPtr.get());
    }
};
static_assert(IRing<OGRRingAdapter>);
}