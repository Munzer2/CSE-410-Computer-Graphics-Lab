#include <windows.h>
#include <GL/glut.h>
#include<bits/stdc++.h>
#include "2005018_classes.hpp" 
#include "bitmap_image.hpp"
using namespace std;


// vector<Object*> objects; // global vector to hold all objects
// vector<PointLight> pointLights; // global vector to hold all point lights
// vector<SpotLight> spotLights; // global vector to hold all spotlights
vector<vector<unsigned char >> pixels; // global vector to hold pixel data for the image

Vect eye(4, 4, 4); // camera position
Vect center(0, 0, 0); // look-at point
Vect up(0, 0, 1); // up vector 
GLdouble  windH = 600, windW = 600; // window height and width
GLint imgH = 600, imgW = 600; // image height and width
GLint recurL, TotPix, TotObj, TotPLS, TotSLS;
GLdouble t_min = 1e6; // minimum t value for intersection 

void _init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, windW / windH, 0.1, 1000.0); // Perspective projection
    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();
}

void _display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(eye.x, eye.y, eye.z,
              center.x, center.y, center.z,
              up.x, up.y, up.z);    
              
    for(int o = 0 ; o < objects.size(); ++o) {
        objects[o]->draw(); 
    }

    glutSwapBuffers();
}

void loadData() {
    /// read a text file called scene.txt
    ifstream file("scene.txt");
    string line; 
    file >> recurL >> TotPix >> TotObj;
    for(int i = 0; i < TotObj; ++i) {
        string obj; 
        file >> obj; 
        if(obj == "sphere") {
            double x, y, z, r ; 
            file >> x >> y >> z >> r;
            Sphere* s = new Sphere(x, y, z, r);
            Vect color; 
            file >> color.x >> color.y >> color.z;
            s->setColor({color.x, color.y, color.z});
            vector< double > coeff(4);
            file >> coeff[0] >> coeff[1] >> coeff[2] >> coeff[3];
            s->setCoefficients(coeff);
            int shine;
            file >> shine;
            s->setShine(shine);
            objects.push_back(s); // add sphere to the objects vector
        }
        else if(obj == "triangle") {
            Vect v1,v2,v3;
            file >> v1.x >> v1.y >> v1.z;
            file >> v2.x >> v2.y >> v2.z;
            file >> v3.x >> v3.y >> v3.z; 
            Triangle * t = new Triangle(v1,v2,v3); 
            Vect color;
            file >> color.x >> color.y >> color.z;
            t->setColor({color.x, color.y, color.z});
            vector < double > coeff(4);
            file >> coeff[0] >> coeff[1] >> coeff[2] >> coeff[3];
            t->setCoefficients(coeff);
            int shine;
            file >> shine; 
            t->setShine(shine);
            objects.push_back(t); // add triangle to the objects vector
        }
        else if(obj == "general") {
            double a, b, c, d, e, f, g, h, i, j;
            Vect ref; 
            double l, w, hgt; 
            file >> a >> b >> c >> d >> e >> f >> g >> h >> i >> j;
            file >> ref.x >> ref.y >> ref.z;
            file >> l >> w >> hgt; 
            Vect color;
            file >> color.x >> color.y >> color.z;
            vector< double > coeff(4);
            file >> coeff[0] >> coeff[1] >> coeff[2] >> coeff[3];
            int shine;
            file >> shine;
            General * gObj = new General(a, b, c, d, e, f, g, h, i, j, ref, l, w, hgt);
            gObj->setColor({color.x, color.y, color.z});
            gObj->setCoefficients(coeff);
            gObj->setShine(shine);
            objects.push_back(gObj); // add general object to the objects vector
        }
        else {
            cerr << "Unknown object type: " << obj << endl; 
            continue;
        }
    }
    file >> TotPLS;
    for(int i = 0 ;i < TotPLS; ++i) {
        Vect pos , col; 
        file >> pos.x >> pos.y >> pos.z;
        file >> col.x >> col.y >> col.z;
        PointLight p(pos, {col.x, col.y, col.z});
        pointLights.push_back(p); // add point light to the pointLights vector
    }

    file >> TotSLS; 
    for(int i = 0; i < TotSLS; ++i) {
        Vect pos, col, dir; 
        double angle; 
        file >> pos.x >> pos.y >> pos.z;
        file >> col.x >> col.y >> col.z;
        file >> dir.x >> dir.y >> dir.z;
        file >> angle; /// cutoff angle in degrees
        SpotLight s(pos, {col.x, col.y, col.z}, dir, angle);
        spotLights.push_back(s); // add spotlight to the spotLights vector
    }
    file.close(); 

    Floor *floor = new Floor(1000, 20);
    floor->setColor({0.5, 0.5, 0.5}); // gray color
    floor->setShine(10); // set shine value
    floor->setCoefficients({0.1, 0.1, 0.1, 0.1}); // set coefficients
    objects.push_back(floor); // add floor to the objects vector 
    return; 
}

void capture() {
    // initialize bitmap image and set background color
    GLdouble planeDist = (windH/2.0) / tan(45.0 * M_PI / 180.0); /// assuming viewAngle is 45 degrees. Will change later
    Vect look = (center - eye).normalize();
    Vect right = (look ^ up).normalize();

    Vect topLeft = eye + look * planeDist + up * (windH / 2.0) - right * (windW / 2.0);
    double du = windW / imgW, dv = windH / imgH;
    topLeft = topLeft + right * (0.5 * du) - up * (0.5 * dv); // center the top left pixel
    int nearest = 0; 
    double t, tMin; 
    pixels.assign(imgH, vector<unsigned char>(imgW * 3));
    for(int i = 0 ; i < imgW; ++i) {
        for(int j = 0 ; j < imgH; ++j) {
            Vect currPix = topLeft + right * (i * du) - up * (j * dv);
            Vect dir = (currPix - eye).normalize();
            tMin = DBL_MAX;
            Ray r(eye, dir);  
            for(int o = 0; o < objects.size(); ++o) {
                t = objects[o]->intersect(&r , objects[o]->color, 0); 
                if(t > 0 && t < tMin) {
                    tMin = t; 
                    nearest = o; 
                }
            }
            double t_min = objects[nearest]->intersect(&r , objects[nearest]->color, 1);
            // update image pixel
            pixels[j][i * 3] = (unsigned char)(objects[nearest]->color[0] * 255);
            pixels[j][i * 3 + 1] = (unsigned char)(objects[nearest]->color[1] * 255);
            pixels[j][i * 3 + 2] = (unsigned char)(objects[nearest]->color[2] * 255);
        }
    } 
    ///save image to a file
    bitmap_image image(imgW, imgH);
    for(int i = 0 ;i < imgH; ++i) {
        for(int j = 0; j < imgW; ++j) {
            image.set_pixel(j, i,
                pixels[i][j * 3 + 2], // Red channel
                pixels[i][j * 3 + 1], // Green channel
                pixels[i][j * 3]      // Blue channel
            );
        } 
    }
    image.save_image("output_11.bmp");
    return; 
}

void _keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case '0':
            capture(); 
            glutPostRedisplay();
            break;
        case '2':
            break;
        default: 
            break;
    }
    return; 
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(windW, windH);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("Ray Tracing");
    loadData(); 

    glutDisplayFunc(_display);
    glutKeyboardFunc(_keyboard);
    // glutSpecialFunc(_specialKeyboard);
    // glutTimerFunc(_animationSpeed, _timer, 0);
    
    _init();
    glutMainLoop();
    return 0;
}