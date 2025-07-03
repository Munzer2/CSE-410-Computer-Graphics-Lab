#include "2005018_classes.hpp"
// #include "vector.h"

// Definitions of global containers
vector<Object*> objects;
vector<PointLight> pointLights;
vector<SpotLight> spotLights;

// ===== Object =====
Object::Object()
    : ref_point(), h(0), w(0), l(0), shine(0) {}

Object::~Object() {}

void Object::draw() {}

void Object::setColor(vector<double> _c) { color = _c; }

void Object::setShine(int _shine) { shine = _shine; }

void Object::setCoefficients(vector<double> _coeff) { coEfficients = _coeff; }

double Object::intersect(Ray* r, vector<double> color, int lvl) {
    return -1.0;
}

// ===== Sphere =====
Sphere::Sphere(double x, double y, double z, double r) {
    ref_point = Vect(x, y, z);
    radius = r;
}

void Sphere::draw() {
    glPushMatrix();
    glTranslatef(ref_point.x, ref_point.y, ref_point.z);
    glColor3f(color[0], color[1], color[2]);
    glutSolidSphere(radius, 20, 20);
    glPopMatrix();
}

double Sphere::intersect(Ray* r, vector<double> color, int lvl) {
    const double EPS = 1e-6;
    Vect dirN = r->dir.normalize();
    Vect oc = r->start - ref_point;
    double t_p = oc.dot(dirN);
    if (t_p < 0 && oc.dot(oc) > radius*radius) return -1.0;
    double d2 = oc.dot(oc) - t_p*t_p;
    if (d2 > radius*radius) return -1.0;
    double t_h = sqrt(radius*radius - d2);
    double t0 = t_p - t_h;
    double t1 = t_p + t_h;
    double t = (t0 > EPS) ? t0 : (t1 > EPS ? t1 : -1.0);
    if (t < 0) return -1.0;
    if (lvl == 0) return t;
    return t;
}

// ===== PointLight =====
PointLight::PointLight(Vect pos, vector<double> c) {
    this->pos = Vect(pos.x, pos.y, pos.z);
    color = c;
}

// ===== SpotLight =====
SpotLight::SpotLight(Vect pos, vector<double> c, Vect _dir, double a)
    : pointLight(pos, c), dir(_dir), angle(a) {}

// ===== Floor =====
Floor::Floor(double floorWidth, double tileWidth) {
    ref_point = Vect(-floorWidth/2, -floorWidth/2, 0);
    l = tileWidth;
}


void Floor::draw() {
    // Implement floor drawing logic here
    GLdouble start_pos = ref_point.x;
    GLint grid = (-ref_point.x * 2) / l;
    for(int i = 0 ; i < grid; ++i) {
        for(int j = 0 ; j < grid; ++j) { 
            GLdouble x = start_pos + i*l, y = start_pos + j*l;
            if((i+j)%2 == 0) {
                glColor3f(1.0f, 1.0f, 1.0f); // white
            } else {
                glColor3f(0.0f, 0.0f, 0.0f); // black
            }

            glBegin(GL_QUADS);
            {
                glVertex3f(x, y, 0.0f);
                glVertex3f(x + l, y, 0.0f);
                glVertex3f(x + l, y + l, 0.0f);
                glVertex3f(x, y + l, 0.0f);
            }
            glEnd(); 
        }
    }
}

// ===== Triangle =====
Triangle::Triangle(Vect _v1, Vect _v2, Vect _v3)
    : v1(_v1), v2(_v2), v3(_v3) {
    normal = (v2 - v1) ^ (v3 - v1);
    normal = normal.normalize();
}

void Triangle::draw() {
    glBegin(GL_TRIANGLES);
    glColor3f(color[0], color[1], color[2]);
    glVertex3f(v1.x, v1.y, v1.z);
    glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v3.x, v3.y, v3.z);
    glEnd();
}

double Triangle::intersect(Ray *r, vector<double> color, int lvl) {
    const double EPS = 1e-6; 

    Vect dir = r->dir.normalize(); 
    Vect edge1 = v2 - v1;
    Vect edge2 = v3 - v1; 
    Vect h = dir ^ edge2;
    if( h.magnitude() < EPS) return -1.0; // Ray is parallel to triangle

    double det = edge1.dot(h);
    double inv_det = 1.0 / det; 
    Vect s = r->start - v1;
    double u = inv_det * s.dot(h);
    if(u < 0 || u > 1) return -1.0; // Outside triangle bounds

    Vect qVec = s ^ edge1;
    double v = inv_det * dir.dot(qVec);
    if(v < 0 || u + v > 1) return -1.0; // Outside triangle bounds

    double t = inv_det * edge2.dot(qVec);
    if(t < EPS) return -1.0; // Intersection behind ray start

    if(lvl == 0) return t; // Return intersection distance for level 0
    return t; 
}

// ===== General Object =====
General::General(double _a, double _b, double _c, double _d,
                 double _e, double _f, double _g, double _h,
                 double _i, double _j, Vect ref, double l, double w, double h)
    : a(_a), b(_b), c(_c), d(_d), e(_e), f(_f), g(_g), h(_h), i(_i), j(_j),
      clipRef(ref), clipLen(l), clipWid(w), clipHgt(h) {}


void General::draw() {
    /// nothing to draw.
}

double General::intersect(Ray* r, vector<double> color, int lvl) {
    const double EPS = 1e-6;
    Vect O = r->start; 
    Vect D = r->dir.normalize();

    double _a = this->a * D.x * D.x + this->b * D.y * D.y + this->c * D.z * D.z +
                this->d * D.x * D.y + this->e * D.x * D.z + this->f * D.y * D.z;
    double _b = 2*(this->a * O.x * D.x + this->b * O.y * D.y + this->c * O.z * D.z ) + 
                this->d * (O.x * D.y + O.y * D.x) + this->e * (O.x * D.z + O.z * D.x) +
                this->f * (O.y * D.z + O.z * D.y) + this->g * D.x + this->h * D.y + this->i * D.z;
    double _c = this->a * O.x * O.x + this->b * O.y * O.y + this->c * O.z * O.z +
                this->d * O.x * O.y + this->e * O.x * O.z + this->f * O.y * O.z +
                this->g * O.x + this->h * O.y + this->i * O.z + this->j;
    double check = _b * _b - 4 * _a * _c;
    if(check < 0) return -1.0; // No intersection

    double t1 = (-_b + sqrt(check)) / (2 * _a);
    double t2 = (-_b - sqrt(check)) / (2 * _a);
    if(t1 < t2 ) swap(t1,t2);

    for(double t : {t1, t2}) {
        if(t < EPS) continue; /// behind ray start
        Vect P = O + D * t; // Intersection point

        if((clipLen > 0 && fabs(P.x - clipRef.x) > clipLen / 2) ||
           (clipWid > 0 && fabs(P.y - clipRef.y) > clipWid / 2) ||
           (clipHgt > 0 && fabs(P.z - clipRef.z) > clipHgt / 2)) {
            continue; // Outside clipping bounds
        }

        if(lvl == 0) {
            return t; // Return intersection distance for level 0
        }
        
    }
    return -1.0; 
}

// ===== Ray =====
Ray::Ray(Vect _start, Vect _dir)
    : start(_start), dir(_dir.normalize()) {}