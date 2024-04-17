//
// Created by grube on 10.01.2022.
//
#pragma once
#include <math.h>
#include <fishnet/Constants.hpp>
#include <fishnet/Radians.hpp>
#include <fishnet/Degrees.hpp>
#include <fishnet/Angle.hpp>

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

};

}
