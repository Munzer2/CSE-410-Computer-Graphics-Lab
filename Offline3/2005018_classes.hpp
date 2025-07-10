#ifndef CLASSES_2005018_H
#define CLASSES_2005018_H

#include <bits/stdc++.h>
#include <GL/glut.h>
#include "vector.hpp"

using namespace std;

// Forward declaration
class Object; 
class PointLight;
class SpotLight;
class Ray;

// Global containers for scene elements
extern vector<Object*> objects;
extern vector<PointLight> pointLights;
extern vector<SpotLight> spotLights;
extern GLint recurL, TotPix, TotObj, TotPLS, TotSLS;

// Base class for all scene objects
class Object {
public:
    Vect ref_point;
    double h, w, l;
    vector<double> color = vector<double>(3);
    vector<double> coEfficients = vector<double>(4);
    int shine;

    Object();
    virtual ~Object();

    virtual void draw();
    void setColor(vector<double> _c);
    void setShine(int _shine);
    void setCoefficients(vector<double> _coeff);
    virtual double intersect(Ray* r, vector<double>& color, int lvl); 
};

/// Some helpful functions
void handleReflection(Ray *r, const Vect & P, Vect & N, vector< double > &color, int lvl, Object * obj);
void handlePointLightsEffects(Ray *r, Vect &P, Object * o, Vect N, vector<double> &color, vector<double> &baseColor);
void handleSpotLightsEffects(Ray *r, Vect &P, Object * o, Vect N, vector<double> &color, vector<double> &baseColor); 
// Sphere object
class Sphere : public Object {
public:
    double radius;

    Sphere(double x, double y, double z, double r);
    void draw() override;
    double intersect(Ray* r, vector<double> &color, int lvl) override;
};

// Point light source
class PointLight {
public:
    Vect pos;
    vector<double> color;

    PointLight(Vect pos, vector<double> c);
    void draw(); 
};

// Spotlight with cutoff angle
class SpotLight {
public:
    PointLight pointLight;
    Vect dir;
    double angle;

    SpotLight(Vect pos, vector<double> c, Vect _dir, double a);
    void draw();
};

// Checkerboard floor
class Floor : public Object {
public:
    Floor(double floorWidth, double tileWidth);
    void draw() override;
    double intersect(Ray* r, vector<double>& color, int lvl) override;
};

// Triangle object
class Triangle : public Object {
public:
    Vect v1, v2, v3;
    Vect normal;

    Triangle(Vect _v1, Vect _v2, Vect _v3);
    void draw() override;
    double intersect(Ray* r, vector<double>& color, int lvl) override;
};

class General : public Object {
public:
    double a, b, c, d, e, f, g, h, i, j;
    Vect clipRef; 
    double clipLen, clipWid, clipHgt;
    General(double _a, double _b, double _c, double _d,
            double _e, double _f, double _g, double _h,
            double _i, double _j, Vect ref, double l, double w, double h);
    void draw() override;
    double intersect(Ray* r, vector<double>& color, int lvl) override;
};

// Ray for tracing
class Ray {
public:
    Vect start;
    Vect dir;

    Ray(Vect _start, Vect _dir);
};

#endif // CLASSES_2005018_H