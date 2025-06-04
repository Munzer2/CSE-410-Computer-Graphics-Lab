#ifdef __linux__
#include <GL/glut.h>
#elif defined(_WIN32) || defined(WIN32)
#include <windows.h>
#include <GL/glut.h>
#elif defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "vector.h" 
#include <cmath>
#include <vector>
#include <iostream>
#define PI acos(-1)
using namespace std;

// GLdouble posX = 400.0, posY = 400.0, posZ = 400.0;
GLdouble init_Speed = 300.0; 
GLint _sz_z = 400.0, _sz_y = 800.0 , _sz_x = 800.0;
GLdouble posX = 980.0 , posY = 980.0, posZ = 480.0;
GLdouble _Speed = 18.0; // Speed of camera movement.
GLdouble _angleChange = 3.0; /// Speed of camera angle change
Vect _camPos(posX, posY, posZ); // Camera position.
Vect _camView(-posX, -posY, -posZ);/// View direction of the camera
Vect _camUp(0.0,0.0,1.0); /// Up vector.
GLdouble _ballRad = 40.0, _ballRotAngle = 0.0;
Vect _ballPos(0.0, 0.0, _ballRad - _sz_z);
Vect _ballVel(200.0,100.0, 200.0), _ballRotAxis(0.0, 0.0, 1.0), _normal(0.0, 0.0, 1.0); /// keeping z-axis as world up initially
vector< Vect > _AllNormals = {Vect(1,0,0),Vect(-1,0,0),Vect(0,1,0),Vect(0,-1,0), Vect(0,0,1),Vect(0,0,-1)};
GLdouble min_x = -_sz_x, min_y = -_sz_y,  min_z = -_sz_z,  max_x = _sz_x,  max_y = _sz_y,  max_z = _sz_z;
GLdouble grav = 9.8;
GLdouble bounce_rest = 0.75;
vector< Vect > vertices; /// for drawing the stripes across the ball
int _animationSpeed = 20;
bool _isSimulating = false; 
bool _isArrow = true;  
GLdouble _currTime = glutGet(GLUT_ELAPSED_TIME); 


// all function declarations
void _init(); 
void _axes(GLdouble val);
void _walls();
void _display();
void _keyBoard(unsigned char key, int x, int y);
void _idle();
void _specialKeyboard(int key, int x, int y);
void _ball();
void _ball(GLint stacks, GLint slices); 
void _genSphereVertices(GLint stacks, GLint slices);
void _updatePhysics(GLdouble dt); 
void _arrow();
void _ballReset();
void _showDisplay(); 
// -----------------------------------------------------------------


void _showDisplay() {
    cout << "--- 3D Bouncing Ball Controls ---\n";
    cout << "Space: Toggle simulation on/off\n";
    cout << "v: Toggle velocity arrow display\n";
    cout << "+/-: Increase/decrease ball speed (when paused)\n";
    cout << "r: Reset ball to random position (when paused)\n\n";
    cout << "--- Camera Contols ---\n"; 
    cout << "Arrow keys: Move camera left/right/forward/backward\n";
    cout << "Page Up/Down: Move camera up/down\n";
    cout << "w/s: Move camera up/down (keeping reference point)\n";
    cout << "1/2: Look left/right\n"; 
    cout << "3/4: Look up/down\n"; 
    cout << "5/6: Tilt camera clockwise/counterclockwise\n";
    cout << "Reset ball with speed: 300\n";
    return;
}

void _init() {


_showDisplay();        

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // black background
    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    _genSphereVertices(32, 32);
    gluPerspective(60.0, 1.0, 1.0, 5000.0); // fov, aspect, near, far
}

void _genSphereVertices(GLint stacks, GLint slices) {
    vertices.clear();  

    for (int i = 0; i <= stacks; ++i) {  
        GLdouble ang1 = PI * i / stacks;
        for (int j = 0; j <= slices; ++j) {  
            GLdouble ang2 = 2 * PI * j / slices;

            GLdouble _x = _ballRad * sin(ang1) * cos(ang2);
            GLdouble _y = _ballRad * sin(ang1) * sin(ang2);
            GLdouble _z = _ballRad * cos(ang1);

            vertices.push_back(Vect(_x, _y, _z));
        }
    }
}


void _updatePhysics(GLdouble dt) {
    Vect _prev = _ballPos;
    vector<bool> _check; 
    _ballVel.z -= (_ballVel.z ? grav*dt*10.0 : 0);
    _ballPos += _ballVel * dt; 
    // cout << _ballPos.z << "\n";
    /// collision along z-axis. Ceiling and floor
    if(_ballPos.z - _ballRad < min_z) {
        _ballPos.z = min_z + _ballRad; 
        _ballVel.z = (-_ballVel.z * bounce_rest);
        if(fabs(_ballVel.z) < 0.01) {
            _ballVel.z = 0.0; 
        }
    }
    else if(_ballPos.z + _ballRad > max_z) {
        _ballPos.z = max_z - _ballRad; 
        _ballVel.z = (-_ballVel.z * bounce_rest);
    }

    ///collision along x-axis
    if(_ballPos.x + _ballRad > max_x) {
        _ballPos.x = max_x - _ballRad;
        _ballVel.x = (-_ballVel.x * bounce_rest);
    }
    else if(_ballPos.x - _ballRad < min_x) {
        _ballPos.x = min_x + _ballRad;
        _ballVel.x = (-_ballVel.x * bounce_rest); 
        if (fabs(_ballVel.x) < 0.01) _ballVel.x = 0;
    }

    ///collision along y-axis
    if(_ballPos.y + _ballRad > max_y) {
        _ballPos.y = max_y - _ballRad;
        _ballVel.y = (-_ballVel.y * bounce_rest);
    }
    else if(_ballPos.y - _ballRad < min_y) {
        _ballPos.y = min_y + _ballRad;
        _ballVel.y = (-_ballVel.y * bounce_rest); 
        if (fabs(_ballVel.y) < 0.01) _ballVel.y = 0;
    }
    Vect _dis = _ballPos - _prev;
    double check = _dis.magnitude();

    /// rotate if value is significant enough
    // cout << check << "\n";
    if(check > 0.01) {
        _ballRotAngle += (check / (2*PI*_ballRad)) * 360.0; 
        _ballRotAngle = fmod(_ballRotAngle, 360.0); /// in degrees. 
        _ballRotAxis = (_normal ^ _dis).normalize();
        // cout << _ballRotAxis.x << " " << _ballRotAxis.y << " " << _ballRotAxis.z << "\n";
    }
}

void _axes(GLdouble val) {
    glLineWidth(2.0f);

    glBegin(GL_LINES);
    {
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-val, 0.0f, 0.0f);
        glVertex3f(val, 0.0f, 0.0f);

        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, -val, 0.0f);
        glVertex3f(0.0f, val, 0.0f);

        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, -val);
        glVertex3f(0.0f, 0.0f, val);
    }
    glEnd();
}

void _arrow() { 
    GLdouble _len = 100.0, _headSz = 12.0 ;
    Vect _tip = _ballPos + ((_ballVel.normalize()) * _len);
    Vect _head3 = (_tip + (_ballVel.normalize()) * _headSz); 
    glLineWidth(3.0); 
    glColor3f(1.0, 1.0, 0.0); 
    glBegin(GL_LINES);
    {
        glVertex3f(_ballPos.x, _ballPos.y, _ballPos.z); 
        glVertex3f(_head3.x, _head3.y, _head3.z); 
    }
    glEnd(); 
    Vect _up(0.0, 0.0, 1.0); 
    Vect _right = (_up ^ _tip).normalize(), _left = (_tip ^ _up).normalize();
    Vect _head1 = (_tip + _right * _headSz), _head2 = (_tip + _left * _headSz);
    glColor3f(1.0f, 0.647f, 0.0f);
    glBegin(GL_TRIANGLES);
    {
        glVertex3f(_head1.x, _head1.y, _head1.z );
        glVertex3f(_head2.x, _head2.y, _head2.z );
        glVertex3f(_head3.x, _head3.y, _head3.z ); 
    }
    glEnd(); 
}

// void _ball() {
//     glPushMatrix();
//     {
//         glTranslatef(_ballPos.x , _ballPos.y, _ballPos.z); 
//         glColor3f(1.0, 0.0, 0.0);
//         glutSolidSphere(_ballRad,30, 30);
//     }
//     glPopMatrix(); 
// }

void _ball(GLint stacks, GLint slices) {
    glPushMatrix();
    glTranslatef(_ballPos.x , _ballPos.y, _ballPos.z);

    ///rotate the ball
    glRotatef(_ballRotAngle, _ballRotAxis.x, _ballRotAxis.y, _ballRotAxis.z); 

    for (int j = 0; j < slices; ++j) {
        if (j % 2 == 0)
            glColor3f(1.0, 0, 0); // Red
        else
            glColor3f(0, 1.0, 0); // Green
    
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= stacks; ++i) {
            int cur = i * (slices + 1) + j;
            int next = cur + 1;  
            glVertex3f(vertices[cur].x, vertices[cur].y, vertices[cur].z);
            glVertex3f(vertices[next].x, vertices[next].y, vertices[next].z);
        }
        glEnd();
    }

    glPopMatrix();
}
void _checkered_floor() {
    GLint grid_sz = 20; 
    GLdouble tile_sz = (2.0 * _sz_x)/ grid_sz; 
    GLdouble start_pos = -_sz_x;  
    for(int i = 0 ; i < grid_sz; ++i) {
        for(int j = 0;  j< grid_sz; ++j) {
            GLdouble _x = start_pos + i * tile_sz, _y = start_pos + j*tile_sz;
            if((i+j)%2) {
                glColor3f(1.0f, 1.0f, 1.0f);
            }
            else  glColor3f(0.0f, 0.0f, 0.0f);
            
            glBegin(GL_QUADS); 
            {
                glVertex3f(_x, _y, -_sz_z);
                glVertex3f(_x + tile_sz, _y, -_sz_z);
                glVertex3f(_x + tile_sz, _y + tile_sz, -_sz_z);
                glVertex3f(_x, _y + tile_sz, -_sz_z);
            }
            glEnd(); 
        }
    }
}

void _walls() {
    glColor3f(128/255.0 , 128/255.0, 204/255.0);  // Light bluish purple
    glBegin(GL_QUADS);
    {
        glVertex3f(-_sz_x, -_sz_y, _sz_z);
        glVertex3f(-_sz_x, _sz_y, _sz_z);
        glVertex3f(_sz_x, _sz_y, _sz_z);
        glVertex3f(_sz_x, -_sz_y, _sz_z);
    }
    glEnd(); 

    // glColor3f(1.2f, 0.6f, 0.8f);  // Light blue. Bottom most wall.
    // glBegin(GL_QUADS);
    // {
    //     glVertex3f(-_sz, -_sz, -_sz);
    //     glVertex3f(_sz, -_sz, -_sz);
    //     glVertex3f(_sz, _sz, -_sz);
    //     glVertex3f(-_sz, _sz, -_sz);
    // }
    // glEnd();

    _checkered_floor();

    // glColor3f(1.0f, 1.0f, 0.0f); // yellow
    glColor3f(204/255.0, 204/255.0, 76/255.0);
    glBegin(GL_QUADS);
    {
        glVertex3f(-_sz_x, -_sz_y,-_sz_z);
        glVertex3f(-_sz_x, _sz_y, -_sz_z);
        glVertex3f(-_sz_x, _sz_y, _sz_z);
        glVertex3f(-_sz_x, -_sz_y, _sz_z);
    }
    glEnd();

    glColor3f(76/255.0 , 204/255.0, 204/255.0);  // Cyan 
    glBegin(GL_QUADS);
    {
        // glVertex3f(_sz, -_sz, -_sz);
        // glVertex3f(_sz, -_sz, _sz);
        // glVertex3f(_sz, _sz, _sz);
        // glVertex3f(_sz, _sz, -_sz);
        glVertex3f(_sz_x, -_sz_y,-_sz_z);
        glVertex3f(_sz_x, -_sz_y, _sz_z);
        glVertex3f(_sz_x, _sz_y, _sz_z);
        glVertex3f(_sz_x, _sz_y, -_sz_z);
    }
    glEnd();
    
    // glColor3f(0.8f, 0.0f, 0.0f);  // Reddish
    glColor3f(204/255.0, 76/255.0, 76/255.0);
    glBegin(GL_QUADS);
    {
        // glVertex3f(-_sz, -_sz, -_sz);
        // glVertex3f(_sz, -_sz, -_sz);
        // glVertex3f(_sz, -_sz, _sz);
        // glVertex3f(-_sz, -_sz, _sz);
        glVertex3f(-_sz_x, -_sz_y,-_sz_z);
        glVertex3f(_sz_x, -_sz_y, -_sz_z);
        glVertex3f(_sz_x, -_sz_y, _sz_z);
        glVertex3f(-_sz_x, -_sz_y, _sz_z);
    }
    glEnd();
    
    glColor3f(76/255.0 , 204/255.0, 76/255.0); /// light green
    glBegin(GL_QUADS);
    {
        // glVertex3f(-_sz, _sz, -_sz);
        // glVertex3f(-_sz, _sz, _sz);
        // glVertex3f(_sz, _sz, _sz);
        // glVertex3f(_sz, _sz, -_sz);
        glVertex3f(-_sz_x, _sz_y,-_sz_z);
        glVertex3f(-_sz_x, _sz_y, _sz_z);
        glVertex3f(_sz_x, _sz_y, _sz_z);
        glVertex3f(_sz_x, _sz_y, -_sz_z);
    }
    glEnd();
}


void _display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt(
        _camPos.x, _camPos.y, _camPos.z, // Camera position
        _camPos.x + _camView.x , _camPos.y + _camView.y , _camPos.z + _camView.z,  // Look-at point
        _camUp.x, _camUp.y, _camUp.z // Up vector
    ); 

    // _axes(1000.0);
    _walls();
    _ball(32, 32);
    if(_isArrow) _arrow();
    // _ball(); 
    glutSwapBuffers();
}


void _ballReset() {
    GLdouble check_x = max_x - _ballRad, check_y = max_y - _ballRad, check_z = max_z - _ballRad; 
    _ballPos.x = ((rand() / (GLdouble)RAND_MAX) * 2 * check_x) - check_x;
    _ballPos.y = ((rand() / (GLdouble)RAND_MAX) * 2 * check_y) - check_y;
    _ballPos.z = ((rand() / (GLdouble)RAND_MAX) * 2 * check_z) - check_z;

    double theta = (rand() / (GLdouble)RAND_MAX) * 2.0 * PI;
    GLdouble phi = (rand()/ (GLdouble)RAND_MAX) * PI - (PI/2.0);
    
    _ballVel = Vect(cos(phi) * cos(theta), cos(phi) * sin(theta), sin(phi));
    _ballVel = (_ballVel.normalize())*init_Speed;
    cout << "Reset ball with speed : " << init_Speed << "\n"; 
}

void _keyBoard(unsigned char key, int x, int y) {
    switch(key)
    {
        case '1':///looking left.
            _camView = _camView.rotate(_angleChange, _camUp);
            break;

        case '2':///looking right.
            _camView = _camView.rotate(-_angleChange, _camUp);
            break;
        case '3': ///looking up
            {
                Vect right = (_camView^_camUp).normalize(); 
                _camView = _camView.rotate(_angleChange,right);
                _camUp = (right ^ _camView).normalize();
                break;
            } 
        case '4': ///looking down
            {
                Vect right = (_camView^_camUp).normalize(); 
                _camView = _camView.rotate(-_angleChange,right);
                _camUp = (right ^ _camView).normalize();
                break;
            }
        case '5': ///tilt counter clockwise.
            {
                _camUp = _camUp.rotate(_angleChange, _camView.normalize());
                break;
            }
        case '6': ///tilt clockwise. 
            {
                _camUp = _camUp.rotate(-_angleChange, _camView.normalize()); 
                break; 
            }
        case 'W':
        case 'w':
            {
                _camPos += _camUp.normalize() * _Speed; 
                _camView -= _camUp.normalize() * _Speed;
                break;
            }
        case 'S':
        case 's':
            {
                _camPos -= _camUp.normalize() * _Speed; 
                _camView += _camUp.normalize() * _Speed; 
                break;
            }
        case 32:
            {
                _isSimulating ^= 1;  
                break;
            } 
        case 'V':
        case 'v':
            {
                _isArrow ^= 1;
                break ;
            }
        case '+':
            {
                GLdouble currSpeed = _ballVel.magnitude(); 
                if(!_isSimulating && currSpeed < 1000.0) {
                    currSpeed += 10.0;
                    _ballVel = (_ballVel.normalize() * currSpeed); 
                    cout << "Speed increased to " << currSpeed << "\n";
                }
                break;
            }
        case '-':
            {
                GLdouble currSpeed = _ballVel.magnitude(); 
                if(!_isSimulating && currSpeed > 310.0) {
                    currSpeed -= 10.0;
                    _ballVel = (_ballVel.normalize() * currSpeed); 
                    cout << "Speed decreased to " << currSpeed << "\n";
                }
                break;
            }  
        case 'R':
        case 'r':
            {
                if(!_isSimulating) _ballReset();
                break;
            }
        default:
            {
                cout << "Unknown key pressed.\n"; 
                break; 
            }
    }
    glutPostRedisplay();
}

void _specialKeyboard(int key, int x, int y) {
    Vect right = (_camView ^ _camUp).normalize();  
    switch(key)
    {
        case GLUT_KEY_UP: //// forward movement.
            _camPos += (_camView.normalize() * _Speed);
            break;
        case GLUT_KEY_DOWN: //// backward movement.
            _camPos -= (_camView.normalize() * _Speed);
            break;
        case GLUT_KEY_LEFT: //// left movement.
            {_camPos -= right * _Speed;
            _camView -= right * _Speed;
            _camUp = (right ^ _camView).normalize(); 
            break;}
        case GLUT_KEY_RIGHT: //// right movement.
            {_camPos += right * _Speed; 
            _camView += right * _Speed; 
            _camUp = (right ^ _camView).normalize(); 
            break;}
        case GLUT_KEY_PAGE_UP: //// up movement
            _camPos += (_camUp.normalize() * _Speed);
            break;
        case GLUT_KEY_PAGE_DOWN: //// down movement
            _camPos -= (_camUp.normalize() * _Speed);
            break;
        // default:
        //     {
        //         cout << "Unknown key pressed.\n"; 
        //         break;
        //     }
    }
    glutPostRedisplay();
}



void _timer(int val) {
    if(_isSimulating) {
        // cout << _camPos.x << " " <<  _camPos.y << " " << _camPos.z << "\n";
        // cout << _ballVel.x << "\n";
        GLdouble next = glutGet(GLUT_ELAPSED_TIME);
        GLdouble dt = (next - _currTime)/ 1000.0;
        dt = min(dt, 100.0);
        _updatePhysics(dt);
        _currTime = next;  
    }
    else _currTime = glutGet(GLUT_ELAPSED_TIME);
    glutPostRedisplay();
    glutTimerFunc(_animationSpeed, _timer, 0);
}

void _idle() {
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("Testing");

    glutDisplayFunc(_display);
    glutKeyboardFunc(_keyBoard);
    glutSpecialFunc(_specialKeyboard);
    glutTimerFunc(_animationSpeed, _timer, 0);
    
    _init();
    glutMainLoop();
    return 0;
}
