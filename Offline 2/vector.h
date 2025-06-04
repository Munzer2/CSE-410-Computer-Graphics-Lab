#pragma once
#include <bits/stdc++.h>
#include <math.h>
using namespace std;

double DegToRad(double angle) {
    return angle * acos(-1) / 180.0;
}

class Vect {
public:
    double x,y,z;
    Vect();
    Vect(double x, double y, double z);
    Vect operator+(const Vect &v) const; 
    Vect operator-(const Vect &v) const;
    Vect operator*(const Vect &v) const;
    Vect operator*(const double &d) const;
    Vect operator+=(const Vect &v);
    Vect operator+=(const double &d);
    Vect operator-=(const Vect &v);
    Vect operator-=(const double &d);
    Vect operator*=(const Vect &v);
    Vect operator*=(const double &d);
    Vect operator^(const Vect &v) const; /// using this as cross product.
    friend istream& operator>>(istream &in, Vect &v);
    double dot(const Vect &v);
    Vect normalize();
    Vect rotate(double angle, Vect axis);
    double magnitude();
}; 

Vect::Vect() : x(0), y(0), z(0) {}

Vect::Vect(double x, double y, double z) : x(x), y(y), z(z) {}

Vect Vect::operator+(const Vect &v) const {
    return Vect(x + v.x, y + v.y, z + v.z);
}

double Vect::magnitude() {
    return sqrt(x*x + y*y + z*z); 
}

Vect Vect::operator-(const Vect &v) const {
    return Vect(x - v.x, y - v.y, z - v.z);
}

Vect Vect::operator*(const Vect &v) const {
    return Vect(x * v.x, y * v.y, z * v.z);
}

Vect Vect::operator*(const double &d) const {
    return Vect(x * d, y * d, z * d);
}

Vect Vect::operator+=(const Vect &v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vect Vect::operator+=(const double &d) {
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

Vect Vect::operator-=(const double &d) {
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

Vect Vect::operator*=(const double &d) {
    x *= d;
    y *= d;
    z *= d;
    return *this;
}

Vect Vect::operator^(const Vect &v) const {
    return Vect(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

istream& operator>>(std::istream &in, Vect &v) {
    in >> v.x >> v.y >> v.z;
    return in;
}

double Vect::dot(const Vect &v) {
    return x * v.x + y * v.y + z * v.z;
}

Vect Vect::normalize() {
    double len = sqrt(x * x + y * y + z * z);
    if (!len) return Vect(0, 0, 0); // Avoid division by zero
    return Vect(x / len, y / len, z / len);
}

Vect Vect::rotate(double angle, Vect axis) {
    ////Using Rodrigues' rotation formula 
    axis = axis.normalize(); 
    double _angle = DegToRad(angle); 
    Vect a = (*this)*cos(_angle);
    Vect b = (axis^(*this))*sin(_angle);
    Vect c = axis*(axis.dot(*this))*(1- cos(_angle));
    return a + b + c; 
}





