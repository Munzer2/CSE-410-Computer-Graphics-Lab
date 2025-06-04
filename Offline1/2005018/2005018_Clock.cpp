/**
 * Advanced OpenGL Demo Application
 *
 * This program demonstrates basic OpenGL functionality including:
 * - Drawing and transformations
 * - Keyboard input (normal and special keys)
 * - Mouse interaction
 * - Animation using both idle and timer functions
 *
 * Use as a learning template for OpenGL with GLUT
 */

// Platform-specific includes
#ifdef __linux__
#include <GL/glut.h> // For Linux systems
#elif defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include <GL/glut.h> // For Windows systems
#elif defined(__APPLE__)
#include <GLUT/glut.h> // For macOS systems
#else
#include <GL/glut.h> // Default fallback
#endif

#include <cstdio>
#include <cmath>
#include <sys/time.h>
#include <iostream>
#define PI acos(-1)
using namespace std;

const GLdouble clockRad = 0.6;

void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
}

void drawCircle(GLdouble rad) {
    glBegin(GL_LINE_LOOP);
    for(int i = 0 ; i < 360 ; ++i) {
        GLdouble theta = (i*PI) / 180.0;
        GLdouble x = rad * cos(theta);
        GLdouble y = rad * sin(theta) ;
        glVertex2f(x, y);
    }
    glEnd(); 
}

void getTime(int &hour, int &min, int &sec, GLdouble &milis) {
    struct timeval _time; 
    gettimeofday(&_time, NULL);
    time_t now = _time.tv_sec; 
    struct tm* _time_info = localtime(&now); 
    hour = _time_info->tm_hour % 12;
    hour = (!hour ? 12 : hour);  
    min = _time_info->tm_min;
    sec = _time_info->tm_sec; 
    milis = _time.tv_usec / 1000.0; 
    // cout << hour << " " << min << " " << sec << " " << milis << "\n"; 
}

void drawSquare(GLdouble size) {
    glBegin(GL_QUADS); 
    {
        glVertex2f(-size/2, -size/2);
        glVertex2f(size/2, -size/2);
        glVertex2f(size/2, size/2);
        glVertex2f(-size/2, size/2);
    }
    glEnd(); 
}

void drawHand(GLdouble _angle, GLdouble len, GLdouble wid)
{
    glPushMatrix();
    glRotatef(_angle, 0.0 , 0.0 , 1.0);
    glLineWidth(wid); 
    glBegin(GL_LINES); 
    {
        glVertex2f(0.0, 0.0);
        glVertex2f(len,0.0); 
    }
    glEnd(); 
    glPushMatrix();
    {
        glTranslatef(clockRad, 0.0, 0.0); 
        glRotatef(45.0,0.0, 0.0, 1.0);
        drawSquare(0.05);
    }
    glPopMatrix(); 
    glPopMatrix();
}



void drawLine(GLdouble strt, GLdouble end) {
    glBegin(GL_LINES);
    {
        glVertex2f(0.0, strt); 
        glVertex2f(0.0, end);
    }
    glEnd(); 
}


void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();


    gluLookAt(
        0.0, 0.0, 3.0,
        0.0, 0.0, 0.0, 
        0.0, 1.0, 0.0  
    );

    glColor3f(1.0f , 1.0f, 1.0f);
    drawCircle(clockRad);

    for (int i = 0; i < 60; i++)
    {
        glPushMatrix();
        glRotatef(i * 6.0f, 0.0f, 0.0f, 1.0f);
        if(i%5 == 0) {
            drawLine(clockRad * 0.80, clockRad * 0.95);
        }
        else drawLine(clockRad * 0.87, clockRad * 0.95);
        glEnd();
        glPopMatrix();
    }   

    int hours, seconds, minutes; 
    GLdouble milis; 

    getTime(hours, minutes, seconds, milis); 
    
    GLdouble  secondFraction = seconds + milis/1000.0;
    GLdouble minuteFraction = minutes + secondFraction/60.0;
    GLdouble hourFraction = hours + minuteFraction/60.0;
    
    GLdouble _second = 90.0 - (secondFraction * 6.0);                    
    GLdouble _min = 90.0 - (minuteFraction * 6.0);
    GLdouble _hour = 90.0 - (hourFraction * 30.0);

    glColor3f(1.0f, 1.0f, 1.0f); 
    drawHand(_hour, clockRad * 0.4, 4.0);

    glColor3f(1.0f, 1.0f, 1.0f); 
    drawHand(_min, clockRad * 0.5, 3.0);

    glColor3f(1.0f, 0.0f, 0.0f); 
    drawHand(_second, clockRad * 0.7, 1.0);

    glutSwapBuffers();
}




void reshape(int width, int height)
{
    if (height == 0)
        height = 1;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspectRatio = (float)width / (float)height;
    gluPerspective(45.0f, aspectRatio, 0.1f, 100.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}


void _idle() {
    glutPostRedisplay();
}



int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitWindowSize(500, 500);
    glutInitWindowPosition(0, 0);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);

    glutCreateWindow("Analog Clock Offline");
    glutDisplayFunc(display);            
    glutReshapeFunc(reshape);
    glutIdleFunc(_idle);     
    init();
    glutMainLoop();

    return 0;
}