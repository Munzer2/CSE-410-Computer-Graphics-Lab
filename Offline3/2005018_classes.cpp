#include "2005018_classes.hpp"
// #include "vector.h"

// Definitions of global containers
vector<Object *> objects;
vector<PointLight> pointLights;
vector<SpotLight> spotLights;
GLint recurL, TotPix, TotObj, TotPLS, TotSLS;

// General functions
void handleReflection(Ray *r, const Vect &P, Vect &N, vector<double> &color, int lvl, Object *obj)
{
    const double EPS = 1e-6;
    if (lvl <= recurL && obj->coEfficients[3] > 0)
    {
        Vect V = (r->dir * -1.0).normalize();
        Vect Rdir = (N * 2 * (N.dot(V)) - V).normalize();
        Ray reflecRay(P + Rdir * EPS, Rdir);

        vector<double> reflectColor(3, 0.0), tmp(3);
        double t_reflec = -1.0;
        int best = -1;
        for (int o = 0; o < objects.size(); ++o)
        {
            if (objects[o] == obj)
                continue; // Skip self
            double t = objects[o]->intersect(&reflecRay, tmp, lvl + 1);
            if (t > 0 && (best < 0 || t < t_reflec))
            {
                t_reflec = t;
                best = o;
                reflectColor = tmp;
            }
        }

        if (best >= 0)
        {
            for (int c = 0; c < 3; ++c)
            {
                color[c] += obj->coEfficients[3] * reflectColor[c];
            }
        }
    }
    return;
}

void handlePointLightsEffects(Ray *r, Vect &P, Object *o, Vect N, vector<double> &color, vector<double> &baseColor)
{
    const double EPS = 1e-6;
    for (auto &p : pointLights)
    {
        Vect LD = (p.pos - P).normalize(); // Light direction
        Ray shadowRay(P + LD * EPS, LD);   /// avoid self-shadowing

        bool inShadow = false;
        double distLight = (p.pos - P).magnitude();
        for (auto &obj : objects)
        {
            if (obj == o)
                continue; // Skip self
            vector<double> tmp(3);
            double t_shadow = obj->intersect(&shadowRay, tmp, 0);
            if (t_shadow > 0 && t_shadow < distLight)
            {
                inShadow = true; // Found an object blocking the light
                break;
            }
        }
        if (inShadow)
            continue; // Skip if in shadow

        double lambert = max(0.0, N.dot(LD)); // Diffuse component
        if (lambert == 0 || lambert < EPS)
            continue;
        for (int c = 0; c < 3; ++c)
        {
            color[c] += p.color[c] * (o->coEfficients[1] * lambert * baseColor[c]); // Diffuse component
        }

        Vect V = (r->dir * -1.0).normalize();        // View direction
        Vect R = (N * 2 * lambert - LD).normalize(); // Reflection vector
        double spec = pow(max(0.0, R.dot(V)), o->shine);
        for (int c = 0; c < 3; ++c)
            color[c] += p.color[c] * o->coEfficients[2] * spec;
    }
    return;
}

void handleSpotLightsEffects(Ray *r, Vect &P, Object *o, Vect N, vector<double> &color, vector<double> &baseColor)
{
    const double EPS = 1e-6;

    for (auto &s : spotLights)
    {
        Vect LD = (s.pointLight.pos - P).normalize(); // Light direction
        double theta = acos(LD.dot(s.dir.normalize())) * 180.0 / M_PI;
        if (theta > s.angle)
            continue; // Outside spotlight cone

        Ray shadowRay(P + N * EPS, LD);
        bool inShadow = false;
        double distLight = (s.pointLight.pos - P).magnitude();
        for (auto &ob : objects)
        {
            if (ob == o)
                continue; // Skip self
            vector<double> tmp(3);
            double t_s = ob->intersect(&shadowRay, tmp, 0);
            if (t_s > 0 && t_s < distLight)
            {
                inShadow = true; // Found an object blocking the light
                break;
            }
        }
        if (inShadow)
            continue; // Skip if in shadow

        double lambert = max(0.0, N.dot(LD)); // Diffuse component

        if (lambert == 0.0 || lambert < EPS)
            continue;
        for (int c = 0; c < 3; ++c)
        {
            color[c] += s.pointLight.color[c] * (o->coEfficients[1] * lambert * baseColor[c]);
        }

        Vect V = (r->dir * -1.0).normalize();        // View direction
        Vect R = (N * 2 * lambert - LD).normalize(); // Reflection vector
        double spec = pow(max(0.0, R.dot(V)), o->shine);
        for (int c = 0; c < 3; ++c)
        {
            color[c] += s.pointLight.color[c] * o->coEfficients[2] * spec;
        }
    }
    return;
}

// ===== Object =====
Object::Object()
    : ref_point(), h(0), w(0), l(0), shine(0) {}

Object::~Object() {}

void Object::draw() {}

void Object::setColor(vector<double> _c) { color = _c; }

void Object::setShine(int _shine) { shine = _shine; }

void Object::setCoefficients(vector<double> _coeff) { coEfficients = _coeff; }

double Object::intersect(Ray *r, vector<double> &color, int lvl)
{
    return -1.0;
}

// ===== Sphere =====
Sphere::Sphere(double x, double y, double z, double r)
{
    ref_point = Vect(x, y, z);
    radius = r;
}

void Sphere::draw()
{
    glPushMatrix();
    glTranslatef(ref_point.x, ref_point.y, ref_point.z);
    glColor3f(color[0], color[1], color[2]);
    glutSolidSphere(radius, 20, 20);
    glPopMatrix();
}

double Sphere::intersect(Ray *r, vector<double> &color, int lvl)
{
    const double EPS = 1e-6;
    Vect dirN = r->dir.normalize();
    Vect oc = r->start - ref_point;
    double t_p = (ref_point - r->start).dot(dirN);
    if (t_p < 0 && oc.dot(oc) > radius * radius)
        return -1.0;
    double d2 = oc.dot(oc) - t_p * t_p;
    if (d2 > radius * radius)
        return -1.0;
    double t_h = sqrt(radius * radius - d2);
    double t0 = t_p - t_h;
    double t1 = t_p + t_h;
    double t = (t0 > EPS) ? t0 : (t1 > EPS ? t1 : -1.0);
    if (t < 0)
        return -1.0;
    if (lvl == 0)
        return t;

    Vect P = r->start + dirN * t;         // Intersection point
    Vect N = (P - ref_point).normalize(); // Normal at intersection point

    vector<double> baseColor = this->color;
    for (int c = 0; c < 3; ++c)
        color[c] = baseColor[c] * coEfficients[0]; // Ambient component

    handlePointLightsEffects(r, P, this, N, color, baseColor);

    handleSpotLightsEffects(r, P, this, N, color, baseColor);

    handleReflection(r, P, N, color, lvl, this);
    return t;
}

// ===== PointLight =====
PointLight::PointLight(Vect pos, vector<double> c)
{
    this->pos = Vect(pos.x, pos.y, pos.z);
    color = c;
}

void PointLight::draw()
{
    glPushMatrix();
    glTranslatef(pos.x, pos.y, pos.z);
    glColor3f(color[0], color[1], color[2]);
    glutSolidSphere(1.0, 10, 10); // Draw a small sphere to represent the light
    glPopMatrix();
}

// ===== SpotLight =====
SpotLight::SpotLight(Vect pos, vector<double> c, Vect _dir, double a)
    : pointLight(pos, c), dir(_dir), angle(a) {}

void SpotLight::draw()
{
    glPushMatrix();
    glTranslatef(pointLight.pos.x, pointLight.pos.y, pointLight.pos.z);
    glColor3f(pointLight.color[0], pointLight.color[1], pointLight.color[2]);
    glutSolidSphere(1.0, 10, 10);
    glPopMatrix();
}
// ===== Floor =====
Floor::Floor(double floorWidth, double tileWidth)
{
    ref_point = Vect(-floorWidth / 2, -floorWidth / 2, 0);
    l = tileWidth;
}

void Floor::draw()
{
    // Implement floor drawing logic here
    GLdouble start_pos = ref_point.x;
    GLint grid = (-ref_point.x * 2) / l;
    for (int i = 0; i < grid; ++i)
    {
        for (int j = 0; j < grid; ++j)
        {
            GLdouble x = start_pos + i * l, y = start_pos + j * l;
            if ((i + j) % 2 == 0)
            {
                glColor3f(1.0f, 1.0f, 1.0f); // white
            }
            else
            {
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

double Floor::intersect(Ray *r, vector<double> &color, int lvl)
{
    const double EPS = 1e-6;
    Vect dir = r->dir.normalize();
    if (fabs(dir.z) < EPS)
        return -1.0; // Ray is parallel to the floor

    double t = (ref_point.z - r->start.z) / dir.z;
    if (t < EPS)
        return -1.0;

    if (lvl == 0)
        return t; // Return intersection distance for level 0

    Vect P = r->start + dir * t;
    if (fabs(P.x) > -ref_point.x || fabs(P.y) > -ref_point.y)
    {
        return -1.0;
    }

    Vect N(0, 0, 1);
    int x = floor((P.x - ref_point.x) / l);
    int y = floor((P.y - ref_point.y) / l);
    vector<double> baseColor = ((x + y) % 2 == 0 ? vector<double>{1.0, 1.0, 1.0} : vector<double>{0.0, 0.0, 0.0});
    for (int c = 0; c < 3; ++c)
    {
        color[c] = baseColor[c] * coEfficients[0]; // Ambient component
    }

    handlePointLightsEffects(r, P, this, N, color, baseColor);

    handleSpotLightsEffects(r, P, this, N, color, baseColor);

    handleReflection(r, P, N, color, lvl, this);

    return t;
}

// ===== Triangle =====
Triangle::Triangle(Vect _v1, Vect _v2, Vect _v3)
    : v1(_v1), v2(_v2), v3(_v3)
{
    normal = (v2 - v1) ^ (v3 - v1);
    normal = normal.normalize();
}

void Triangle::draw()
{
    glBegin(GL_TRIANGLES);
    glColor3f(color[0], color[1], color[2]);
    glVertex3f(v1.x, v1.y, v1.z);
    glVertex3f(v2.x, v2.y, v2.z);
    glVertex3f(v3.x, v3.y, v3.z);
    glEnd();
}

double Triangle::intersect(Ray *r, vector<double> &color, int lvl)
{
    const double EPS = 1e-6;

    Vect dir = r->dir.normalize();
    Vect edge1 = v2 - v1;
    Vect edge2 = v3 - v1;
    Vect h = dir ^ edge2;
    if (h.magnitude() < EPS)
        return -1.0; // Ray is parallel to triangle

    double det = edge1.dot(h);
    if (fabs(det) < EPS)
        return -1.0;
    double inv_det = 1.0 / det;
    Vect s = r->start - v1;
    double u = inv_det * s.dot(h);
    if (u < 0 || u > 1)
        return -1.0; // Outside triangle bounds

    Vect qVec = s ^ edge1;
    double v = inv_det * dir.dot(qVec);
    if (v < 0 || u + v > 1)
        return -1.0; // Outside triangle bounds

    double t = inv_det * edge2.dot(qVec);
    if (t < EPS)
        return -1.0; // Intersection behind ray start

    if (lvl == 0)
        return t; // Return intersection distance for level 0

    Vect P = r->start + dir * t; // Intersection point
    vector<double> baseColor = this->color;

    Vect N = this->normal; // Normal at intersection point
    if (N.dot(dir) > 0)
        N = N * -1.0; // Ensure normal points towards ray
    for (int c = 0; c < 3; ++c)
    {
        color[c] = baseColor[c] * coEfficients[0]; // Ambient component
    }
    handlePointLightsEffects(r, P, this, N, color, baseColor);
    handleSpotLightsEffects(r, P, this, N, color, baseColor);
    handleReflection(r, P, N, color, lvl, this);
    return t;
}

// ===== General Object =====
General::General(double _a, double _b, double _c, double _d,
                 double _e, double _f, double _g, double _h,
                 double _i, double _j, Vect ref, double l, double w, double h)
    : a(_a), b(_b), c(_c), d(_d), e(_e), f(_f), g(_g), h(_h), i(_i), j(_j),
      clipRef(ref), clipLen(l), clipWid(w), clipHgt(h) {}

void General::draw()
{
    /// nothing to draw.
}

double General::intersect(Ray *r, vector<double> &color, int lvl)
{
    const double EPS = 1e-6;
    Vect O = r->start;
    Vect D = r->dir.normalize();

    double _a = a * D.x * D.x + b * D.y * D.y + c * D.z * D.z + d * D.x * D.y + e * D.x * D.z + f * D.y * D.z;
    double _b =
        2 * (a * O.x * D.x + b * O.y * D.y + c * O.z * D.z) + d * (O.x * D.y + O.y * D.x) + e * (O.x * D.z + O.z * D.x) + f * (O.y * D.z + O.z * D.y) + g * D.x + h * D.y + i * D.z;

    double _c =
        a * O.x * O.x + b * O.y * O.y + c * O.z * O.z + d * O.x * O.y + e * O.x * O.z + f * O.y * O.z + g * O.x + h * O.y + i * O.z + j;
    double check = _b * _b - 4 * _a * _c;
    if (check < 0)
        return -1.0; // No intersection

    double t1 = (-_b + sqrt(check)) / (2 * _a);
    double t2 = (-_b - sqrt(check)) / (2 * _a);
    if (t1 > t2)
        swap(t1, t2);

    double t_hit = -1.0;
    for (double t : {t1, t2})
    {
        if (t < EPS)
            continue;
        Vect P = O + D * t; // Intersection point

        if(clipLen > 0.0) {
            if(P.x < clipRef.x || P.x > clipRef.x + clipLen) continue; 
        }

        if(clipWid > 0.0 ) {
            if(P.y < clipRef.y || P.y > clipRef.y + clipWid) continue;
        }

        if(clipHgt > 0.0) {
            if(P.z < clipRef.z || P.z > clipRef.z + clipHgt) continue;
        }
        t_hit = t;
        break;
    }
    if (t_hit < 0 || lvl == 0)
        return t_hit;

    Vect P = O + D * t_hit; // Intersection point
    Vect N = Vect(
                 2 * a * P.x + d * P.y + e * P.z + g,
                 d * P.x + 2 * b * P.y + f * P.z + h,
                 e * P.x + f * P.y + 2 * c * P.z + i)
                 .normalize();
    if (N.dot(D) > 0)
        N = N * -1.0; // Ensure normal points towards ray

    vector<double> baseColor = this->color;
    for (int c = 0; c < 3; ++c)
    {
        color[c] = baseColor[c] * coEfficients[0]; // Ambient component
    }

    handlePointLightsEffects(r, P, this, N, color, baseColor);

    handleSpotLightsEffects(r, P, this, N, color, baseColor);

    handleReflection(r, P, N, color, lvl, this);
    return t_hit;
}

// ===== Ray =====
Ray::Ray(Vect _start, Vect _dir)
    : start(_start), dir(_dir.normalize()) {}