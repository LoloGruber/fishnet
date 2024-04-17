//
// Created by grube on 10.01.2022.
//
#pragma once
#include <math.h>
#include <fishnet/Constants.hpp>
#include <fishnet/Radians.hpp>
#include <fishnet/Degrees.hpp>
#include <fishnet/Angle.hpp>
#include <fishnet/GeometryObject.hpp>
#include <fishnet/Ring.hpp>
#include <fishnet/Polygon.hpp>
#include <algorithm>
#include <ranges>

namespace fishnet {

/**
 * Util class for geographic computations
 */

using namespace math;
class WGS84Ellipsoid {

private:
    static double cos2(double angle) {
        return pow(cos((math::PI / 180) * angle), 2);
    }

    static double cos2(Degrees angle){
        return pow(angle.cos(), 2);
    }

    static double sin2(double angle) {
        return pow(sin((math::PI / 180) * angle), 2);
    }


    static double sin2(Degrees angle){
        return pow(angle.sin(), 2);
    }

    static auto projectToSinusoidal(geometry::IRing auto const & ring) noexcept {
        double lat_dist = math::DEG_TO_RAD * radiusInMeter;
        return geometry::Ring(ring.getPoints() | std::views::transform([&lat_dist](const auto & point){
            auto lon = point.x;
            auto lat = point.y;
            return geometry::Vec2D(lon * lat_dist * Radians(lat).cos(), lat * lat_dist);
        })); // https://stackoverflow.com/questions/4681737/how-to-calculate-the-area-of-a-polygon-on-the-earths-surface-using-python
    }

public:
    constexpr static double flattening = 1.0 / 298.257223563; // Flattening of the Earth: https://en.wikipedia.org/wiki/Earth_ellipsoid
    constexpr static double radiusInMeter = 6378137; //Earth radius in m;

    /**
     *
     * @param lambdaA longitude of point A
     * @param phiA latitude of point A
     * @param lambdaB longitude of point B
     * @param phiB latitude of point B
     * @param exact applies additional correction for the distances
     * For further reading consider: https://de.wikipedia.org/wiki/Orthodrome
     * @return distance between A and B in meters
     */
    static double distance(double lambdaA, double phiA, double lambdaB, double phiB, bool exact = true) {
        Degrees F = Degrees((phiA + phiB) / 2);
        Degrees G = Degrees((phiA - phiB) / 2);
        Degrees l = Degrees((lambdaA - lambdaB) / 2);
        double S = sin2(G) * cos2(l) + cos2(F) * sin2(l);
        double C = cos2(G) * cos2(l) + sin2(F) * sin2(l);
        Radians w = Radians::atan(sqrt(S / C));
        double distance = 2 * w.getAngleValue() * radiusInMeter;
        if (not exact) {
            return distance;
        } else {
            double T = sqrt(S * C) / w.getAngleValue();
            double h1 = (3 * T - 1) / (2 * C);
            double h2 = (3 * T + 1) / (2 * S);
            double correction = (1 + flattening * h1 * sin2(F) * cos2(G) -
                                 flattening * h2 * cos2(F) * sin2(G));
            return distance * correction;
        }
    }

    static double distance(geometry::IPoint auto const & p, geometry::IPoint auto const & q,bool exact = true)noexcept{
        return distance(p.x, p.y, q.x, q.y, exact);
    }

    /**
     * Calculate the area of a Polygon in m² by projection
     * @param polygon source polygon
     * @return area in m²
     */
    static double area(geometry::IPolygon auto const & polygon)noexcept{
        return geometry::Polygon(projectToSinusoidal(polygon.getBoundary()),std::views::transform(polygon.getHoles(),[](const auto & ring){ return projectToSinusoidal(ring);})).area();
    }

    static double area(geometry::IRing auto const & ring) noexcept {
        return projectToSinusoidal(ring).area();
    }
};

}
