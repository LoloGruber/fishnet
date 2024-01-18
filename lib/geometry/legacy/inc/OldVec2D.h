// //
// // Created by grube on 06.01.2022.
// //

// #ifndef BACHELORARBEIT_VEC2D_H
// #define BACHELORARBEIT_VEC2D_H


// #include <ogr_geometry.h>
// /**
//  * Util class for geometric computations in the two-dimensional space
//  */
// class Vec2D {
// public:
//     double x;
//     double y;
//     constexpr static const double epsilon = 0.00000001;

//     Vec2D(double x, double  y);

//     Vec2D() = default;

// //    Vec2D(){
// //        x = 0.0;
// //        y = 0.0;
// //    };

//     Vec2D(Vec2D &other){
//         this->x = other.x;
//         this->y = other.y;
//     }

//     Vec2D(const Vec2D &other){
//         this->x = other.x;
//         this->y = other.y;
//     }

//     double dot(const Vec2D & other) const;

//     double cross(const Vec2D & other) const;

//     Vec2D operator*(double scalar) const;

//     Vec2D operator+(const Vec2D &other) const;

//     Vec2D operator/(double scalar) const;

//     Vec2D operator-(const Vec2D &other) const;

//     bool operator==(const Vec2D & other) const;

//     bool operator!=(const Vec2D &other) const;

//     bool isParallel(const Vec2D & other) const;

//     double det(const Vec2D & other) const;

//     Vec2D orthogonal() const;

//     Vec2D normalize() const;

//     double length() const;

//     /**
//      * @return OGRPoint representation of the this Vec2D
//      */
//     OGRPoint * toOGRPoint() const;

//     /**
//      * Computes the angle (counterclockwise) for the direction vector with reference to the reference vector, starting rotation on x-axis
//      * @param reference for the angle
//      * @return
//      */
//     double angle(const Vec2D & reference) const;

//     /**
//      * Calculates the angle with the previous method (starting from x-axis and performing a counterclockwise rotation)
//      * then rotates the angle by angleRotate
//      * @param reference
//      * @param angleRotate
//      * @return
//      */
//     double angle(const Vec2D & reference, double angleRotate) const;

//     /**
//      * Returns clockwise angle of this vector with reference to <reference>, rotated by angleRotate to infer the resulting bearing from the current position
//      * @param reference
//      * @param angleRotate
//      * @return
//      */
//     double bearing(const Vec2D & reference, double angleRotate)const;

//     double distance(const Vec2D & other) const;

//     std::string toString() const;

//     /**
//      * Hash Function of a Vector
//      */
//     struct Hash{
//         std::size_t operator()(const Vec2D & obj) const{
//             std::size_t xHash = std::hash<double>()(obj.x);
//             std::size_t yHash = std::hash<double>()(obj.y);
//             return xHash ^ yHash;
//         };
//         std::size_t operator()(Vec2D & obj)const{
//             std::size_t xHash = std::hash<double>()(obj.x);
//             std::size_t yHash = std::hash<double>()(obj.y);
//             return xHash ^ yHash;
//         };
//     };

// };


// #endif //BACHELORARBEIT_VEC2D_H
