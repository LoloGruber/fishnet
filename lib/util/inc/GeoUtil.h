//
// Created by grube on 10.01.2022.
//

#ifndef BACHELORARBEIT_GEOUTIL_H
#define BACHELORARBEIT_GEOUTIL_H

#include <math.h>

/**
 * Util class for geographic compuations
 */
struct GeoUtil{
    constexpr static double f = 1.0 / 29857223563.0; //"Abplattung" der Erde
    constexpr static double radius = 6378137; //Earth radius in m;
    constexpr static double PI = 3.14159265358979323846;

    static double cos2(double angle) {
        return pow(cos((GeoUtil::PI / 180) * angle), 2);
    }

    static double sin2(double angle){
        return pow(sin((GeoUtil::PI / 180) * angle), 2);
    }
    /**
     *
     * @param p Vec2D(x,y) with x=Longitude and y=Latitude
     * @param q --
     * @return distance between p and q on earths surface in meters
     */
    static double distance(double px,double py, double qx,double qy, bool approx =false){ //https://de.wikipedia.org/wiki/Orthodrome
        double F = (py + qy) / 2;
        double G = (py - qy) / 2;
        double l = (px - qx) / 2;

        double S = sin2(G) * cos2(l) + cos2(F) * sin2(l);
        double C = cos2(G) * cos2(l) + sin2(F) * sin2(l);
        //double S = pow(sin(G), 2) * pow(cos(l), 2) + pow(cos(F), 2) * pow(sin(l), 2);
        //double C = pow(cos(G), 2) * pow(cos(l), 2) + pow(sin(F), 2) * pow(sin(l), 2);

        double w = atan(sqrt(S / C));

        double D = 2 * w * radius;
        if (approx) {
            return D;
        } else {
            double T = sqrt(S * C) / w;
            double h1 = (3 * T - 1) / 2 * C;
            double h2 = (3 * T + 1) / 2 * S;
            double correction = (1 + f * h1 * pow(sin(F), 2) * pow(cos(G), 2) -
                                 f * h2 * pow(cos(F), 2) * pow(sin(G), 2));
            return D * correction;
        }
    }
};
#endif //BACHELORARBEIT_GEOUTIL_H