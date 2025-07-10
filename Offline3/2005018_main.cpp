#include <windows.h>
#include <GL/glut.h>
#include<bits/stdc++.h>
#include "2005018_classes.hpp" 
#include "bitmap_image.hpp"
using namespace std;


vector<vector<unsigned char >> pixels; // global vector to hold pixel data for the image

Vect eye(60, 60, 60); // camera position
Vect look = Vect(-eye.x, -eye.y, -eye.z); // look-at point
Vect up(0, 0, 1); // up vector 
GLint  windH = 600, windW = 600; // window height and width
GLint imgH = 600, imgW = 600; // image height and width
GLdouble angleChange = 5.0, camSpeed = 20.0; // angle change for rotation
GLdouble fovY = 60.0 , znear = 1, zfar = 1000.0; // field of view, near and far plane distances
GLdouble t_min = 1e6; // minimum t value for intersection 

void _init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(fovY, (GLdouble)windW / (GLdouble)windH, znear, zfar); // Perspective projection
    // glMatrixMode(GL_MODELVIEW);
    // glLoadIdentity();
}

void _axes() {
    glBegin(GL_LINES);
    {
        glColor3f(1.0f, 0.0f, 0.0f); // Red for X-axis
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(100.0f, 0.0f, 0.0f);

        glColor3f(0.0f, 1.0f, 0.0f); // Green for Y-axis
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 100.0f, 0.0f);

        glColor3f(0.0f, 0.0f, 1.0f); // Blue for Z-axis
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, 100.0f);
    }
    glEnd();
}

void _display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(eye.x, eye.y, eye.z,
              eye.x + look.x, eye.y + look.y, eye.z + look.z,
              up.x, up.y, up.z);  
    
    // _axes(); 
              
    for(int o = 0 ; o < objects.size(); ++o) {
        objects[o]->draw(); 
    }

    for(int p = 0 ; p < pointLights.size(); ++p) {
        pointLights[p].draw();
    }

    for(int s = 0 ; s < spotLights.size(); ++s) {
        spotLights[s].draw();
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
    floor->setColor({0.5, 0.5, 0.5}); 
    floor->setShine(10); 
    floor->setCoefficients({0.1, 0.8, 0.0, 0.4});
    objects.push_back(floor); 

    imgH = imgW = TotPix;
    windW = windH = TotPix;
    return; 
}

void capture() {
    double planeDist = ((GLdouble)windH / 2.0) / tan(DegToRad(fovY) / 2.0); 
    double halfH = (GLdouble)windH/2.0;
    double halfW =(GLdouble)windW/2.0;  
    Vect _look = (look).normalize();
    Vect right = (_look ^ up).normalize();
    Vect topLeft = eye + (_look * planeDist) + (up * halfH) -(right * halfW);
    double du = (2.0 * halfW) / (GLdouble)imgW;
    double dv = (2.0 * halfH) / (GLdouble)imgH;
    topLeft = topLeft + (right * 0.5 * du) - (up * 0.5 * dv) ;
    double tMin, t;
    bitmap_image image(imgW, imgH);
    vector< double > col(3); 
    for(int i = 0 ; i < imgW; ++i) {
        for(int j = 0 ; j < imgH; ++j) {
            Vect currPix = topLeft + right * (i * du) - up * (j * dv);
            Vect dir = (currPix - eye).normalize();
            tMin = DBL_MAX;
            int nearest = -1;
            Ray *r = new Ray(eye, dir);  
            for(int o = 0; o < objects.size(); ++o) {
                t = objects[o]->intersect(r , col , 0); 
                if(t > 0 && t < tMin && t < zfar && t > znear) {
                    tMin = t; 
                    nearest = o; 
                }
            }
            
            if(nearest == -1) { /// black pixel
                image.set_pixel(i, j, 0, 0, 0); // Set pixel to black; 
                delete r; 
                continue;
            }
            objects[nearest]->intersect(r , col, 1);
            // update image pixel
            auto gamma = 1.0/2.2; // Gamma correction value
            double _r = clamp(col[0], 0.0, 1.0);
            double _g = clamp(col[1], 0.0, 1.0); 
            double _b = clamp(col[2], 0.0, 1.0);
            // image.set_pixel(i, j,
            //     (unsigned char)(pow(_r, gamma)* 255), // Red channel
            //     (unsigned char)(pow(_g, gamma)* 255), // Green channel
            //     (unsigned char)(pow(_b, gamma)* 255)  // Blue channel
            // );
            image.set_pixel(i, j,
                (unsigned char)(_r* 255), // Red channel
                (unsigned char)(_g* 255), // Green channel
                (unsigned char)(_b* 255)  // Blue channel
            );
            delete r;
            col[0] = col[1] = col[2] = 0.0; // Reset color for next pixel
        }
    } 
    
    // for(int i = 0 ;i < imgH; ++i) {
    //     for(int j = 0; j < imgW; ++j) {
    //         image.set_pixel(j, i,
    //             pixels[i][j * 3], // Red channel
    //             pixels[i][j * 3 + 1], // Green channel
    //             pixels[i][j * 3 + 2]      // Blue channel
    //         );
    //     } 
    // }
    image.save_image("output_11.bmp");
    return; 
}

void _keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case '0':
            capture(); 
            glutPostRedisplay();
            break;
        case '1':
            look = look.rotate(angleChange, up); 
            break;
        case '2':
            look = look.rotate(-angleChange, up); 
            break;
        case '3':
            {
                Vect right = (look ^ up).normalize(); 
                look = look.rotate(angleChange, right);
                up = (right ^ look).normalize();
                break;
            }
        case '4':
            {
                Vect right = (look ^ up).normalize(); 
                look = look.rotate(-angleChange, right);
                up = (right ^ look).normalize();
                break;
            }
        case '5': 
            up = up.rotate(angleChange, look.normalize());
            break; 
        case '6':
            up  = up.rotate(-angleChange, look.normalize());
            break;
        case 'q':
            exit(0);  
        default: 
            break;
    }
    glutPostRedisplay();
    return; 
}

void _specialKeyboard(int key, int x, int y) {
    Vect right = (look ^ up).normalize();
    switch(key) {
        case GLUT_KEY_UP:
            eye += look.normalize() * camSpeed; // move camera forward
            break;
        case GLUT_KEY_DOWN:
            eye -= look.normalize() * camSpeed; // move camera backward
            break;
        case GLUT_KEY_LEFT:
            eye -= right * camSpeed; 
            break; 
        case GLUT_KEY_RIGHT:
            eye += right * camSpeed; 
            break;
        case GLUT_KEY_PAGE_DOWN:
            eye -= up.normalize() * camSpeed; 
            break;
        case GLUT_KEY_PAGE_UP:
            eye += up.normalize() * camSpeed;
            break;
        default:
            break;
    }
    glutPostRedisplay();
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    loadData(); 
    glutInitWindowSize(windW, windH);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("Ray Tracing");

    glutDisplayFunc(_display);
    glutKeyboardFunc(_keyboard);
    glutSpecialFunc(_specialKeyboard);
    // glutTimerFunc(_animationSpeed, _timer, 0);
    
    _init();
    glutMainLoop();
    return 0;
}