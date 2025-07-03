#include "vector.hpp"
#include <cmath>

GLdouble DegToRad(GLdouble angle) {
    return angle * acos(-1) / 180.0;
}

Vect::Vect() : x(0), y(0), z(0) {}

Vect::Vect(GLdouble x, GLdouble y, GLdouble z) : x(x), y(y), z(z) {}

Vect Vect::operator+(const Vect &v) const {
    return Vect(x + v.x, y + v.y, z + v.z);
}

GLdouble Vect::magnitude() {
    return sqrt(x*x + y*y + z*z); 
}

Vect Vect::operator-(const Vect &v) const {
    return Vect(x - v.x, y - v.y, z - v.z);
}

Vect Vect::operator*(const Vect &v) const {
    return Vect(x * v.x, y * v.y, z * v.z);
}

Vect Vect::operator*(const GLdouble &d) const {
    return Vect(x * d, y * d, z * d);
}

Vect Vect::operator+=(const Vect &v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vect Vect::operator+=(const GLdouble &d) {
    x += d;
    y += d;
    z += d;
    return *this;
}

Vect Vect::operator-=(const Vect &v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

Vect Vect::operator-=(const GLdouble &d) {
    x -= d;
    y -= d;
    z -= d;
    return *this;
}

Vect Vect::operator*=(const Vect &v) {
    x *= v.x;
    y *= v.y;
    z *= v.z;
    return *this;
}

Vect Vect::operator*=(const GLdouble &d) {
    x *= d;
    y *= d;
    z *= d;
    return *this;
}

Vect Vect::operator^(const Vect &v) const {
    return Vect(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

GLdouble Vect::dot(const Vect &v) {
    return x * v.x + y * v.y + z * v.z;
}

Vect Vect::normalize() {
    GLdouble len = sqrt(x * x + y * y + z * z);
    if (!len) return Vect(0, 0, 0); // Avoid division by zero
    return Vect(x / len, y / len, z / len);
}

Vect Vect::rotate(GLdouble angle, Vect axis) {
    ////Using Rodrigues' rotation formula 
    axis = axis.normalize(); 
    GLdouble _angle = DegToRad(angle); 
    Vect a = (*this)*cos(_angle);
    Vect b = (axis^(*this))*sin(_angle);
    Vect c = axis*(axis.dot(*this))*(1- cos(_angle));
    return a + b + c; 
}