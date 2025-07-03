#include <GL/gl.h>
#include <math.h>


GLdouble DegToRad(GLdouble angle); 

class Vect {
public:
    GLdouble x,y,z;
    Vect();
    Vect(GLdouble x, GLdouble y, GLdouble z);
    Vect operator+(const Vect &v) const; 
    Vect operator-(const Vect &v) const;
    Vect operator*(const Vect &v) const;
    Vect operator*(const GLdouble &d) const;
    Vect operator+=(const Vect &v);
    Vect operator+=(const GLdouble &d);
    Vect operator-=(const Vect &v);
    Vect operator-=(const GLdouble &d);
    Vect operator*=(const Vect &v);
    Vect operator*=(const GLdouble &d);
    Vect operator^(const Vect &v) const; /// using this as cross product.
    GLdouble dot(const Vect &v);
    Vect normalize();
    Vect rotate(GLdouble angle, Vect axis);
    GLdouble magnitude();
}; 







