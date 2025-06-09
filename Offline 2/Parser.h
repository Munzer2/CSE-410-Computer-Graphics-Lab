#pragma once
#include<bits/stdc++.h>
#include "Matrix.h"
#include "vector.h"
#define PI acos(-1.0)
using namespace std;

class Parser {
    public:
    vector< Vect > cam; 
    vector< double > perspect; //// fovY, aspectRatio , near, far;
    ifstream _in; 
    ofstream _out, _out2, _out3, _out4;
    stack< Matrix > _stk; 
    Matrix ViewTrans, Project;
    string OutDir; 
    int ScWidth, ScHeight;
    double X_left_limit, Y_bottom_limit, Z_front_limit, Z_rear_limit, dx, dy;
    vector<vector<vector< unsigned char >>> Frame;
    vector< vector< double >> Zbuffer;

    Parser(string in, string out, string config); 
    void _Translate(vector< double > amm);
    void _Rotate(double angle, vector< double > a); 
    void _Scale(vector< double > scales); 
    void _Push(); 
    void _Pop();
    void _Parse(); 
    vector< double > _applyTransformation(vector< double > coord);
    void _drawTriangle(vector< vector< double >> coords); 
    void PrintCamAndPers();
    void computeViewTransMatrix(); 
    void computeProjectionMatrix(); 
    void ProcessStage2(string Outstage2, string in); 
    void ProcessStage3(string Outstage3, string in); 
    void ProcessStage4(string z_buffer, string in); 
    void ReadConfig(string config); 
    void Initialize_Frame_Zbuffer(); 
    void rasterizeTriangle(vector< vector< double >> &tri);
};


void Parser::computeProjectionMatrix() { 
    double fovY = perspect[0] * PI / 180.0;
    double t = perspect[2] * tan(fovY / 2.0);
    double r = t * perspect[1];
    Project._matrix[0][0] = perspect[2] / r;
    Project._matrix[1][1] = perspect[2] / t;
    Project._matrix[2][2] = -(perspect[3] + perspect[2]) / (perspect[3] - perspect[2]);
    Project._matrix[2][3] = -(2 * perspect[3] * perspect[2]) / (perspect[3] - perspect[2]);
    Project._matrix[3][2] = -1.0;
    return;
}


void Parser::rasterizeTriangle(vector< vector< double >> &tri) {
    vector< pair< int, int >> px(3); 
    vector< double > z(3);
    for(int i = 0 ;i < 3; ++i) {
        double _x = tri[i][0] , _y = tri[i][1], _z = tri[i][2];
        double x_ws = X_left_limit + (_x + 1.0) * 0.5 * (-X_left_limit * 2.0);
        double y_ws = Y_bottom_limit + (_y + 1.0) * 0.5 * (-Y_bottom_limit * 2.0);
        int col = clamp((int)((x_ws - X_left_limit) / dx), 0, ScWidth - 1);
        int row = clamp((int)((-Y_bottom_limit - y_ws) / dy), 0, ScHeight - 1);
        px[i] = {row, col};
        z[i] = _z ;  
        // cout << row << " " << col << "\n"; 
    }
    // cout << "\n";
    int min_y = ScHeight - 1, min_x = ScWidth - 1, max_x = 0, max_y = 0; 
    for(auto &p : px) {
        min_y = min(min_y, p.first); 
        min_x = min(min_x, p.second);
        max_x = max(max_x, p.second);
        max_y = max(max_y, p.first);
    }

    ///Assign a color.
    mt19937 rng(12345);
    uniform_int_distribution<int> dist(0, 255);
    vector< unsigned char > color = { (unsigned char)dist(rng), (unsigned char )dist(rng), (unsigned char)dist(rng)};
    auto check = [&](int v1, int v2, int x, int y) {
        return (px[v2].second - px[v1].second) * (x - px[v1].first) 
                - (px[v2].first - px[v1].first) * (y - px[v1].second); 
    };

    auto tri_area = check(0, 1, px[2].first, px[2].second);
    for(int y = min_y; y <= max_y; ++y) {
        for( int x = min_x ; x <= max_x ; ++x) {
            double c1 = check(0,1,x,y) * 1.0 / tri_area ;
            double c2 = check(1,2,x,y) * 1.0 / tri_area ;
            double c3 = check(2,0,x,y) * 1.0 / tri_area ;
            if(c1 >= 0 && c2 >= 0 && c3 >= 0) {
                ///pixel is inside the triangle.
                double z_val = c1 * z[0] + c2 * z[1] + c3 * z[2];
                if(z_val < Zbuffer[y][x]) {
                    Zbuffer[y][x] = z_val; 
                    Frame[y][x] = color; 
                }
            } 
        }
    }
}

void Parser::ProcessStage4(string Outstage4, string in) {
    _out4.open(Outstage4);
    _out4 << fixed << setprecision(7);
    ifstream _in4(in);
    if(!_in4.is_open()) {
        cout << "No such input file for stage 4.\n";
        exit(1);
    }
    vector< vector < double >> tri;
    string line;
    // cout << "Stage-4:\n";
    while(getline(_in4, line)) {
        if(line.empty()) {
            if(tri.size() == 3) {
                rasterizeTriangle(tri);
                tri.clear();
            }
            else {
                ////edge case.
            }
        }
        else {
            istringstream iss(line);
            vector< double > coords(3);
            for(int i = 0 ; i < 3; ++i) {
                iss >> coords[i];
            }
            tri.push_back(coords);
        }
    }
    if (tri.size() == 3) rasterizeTriangle(tri); /// If file doesnt end with an empty line.

    _in4.close();

    /// Now to save the frame and z-buffer...... 
    return;
}

void Parser::ProcessStage3(string Outstage3, string in) {
    _out3.open(Outstage3);
    _out3 << fixed << setprecision(7);
    ifstream _in3(in);  
    if(!_in3.is_open()) {
        cout << "No such input file for stage 3.\n";
        exit(1);
    }
    string line; 
    while(getline(_in3, line)) {
        if(line.empty()) {
            _out3 << "\n"; 
            continue; 
        }
        istringstream iss(line);
        vector< double > coords(3);
        for(int i = 0 ; i < 3; ++i) {
            iss >> coords[i];
        }
        coords.push_back(1.0); // Homogeneous coordinate
        coords = Project.apply(coords); 
        for(int i = 0 ; i < 4; ++i) {
            coords[i] /= coords[3];
        }
        for(int i =0 ;i < 3 ; ++i ) _out3 << coords[i] << " ";
        _out3 << "\n"; 
    }
    _in3.close(); 
    _out3.close(); 
    ProcessStage4(OutDir + "/z_buffer.txt",OutDir + "/stage3.txt"); 
    return; 
}

void Parser::ProcessStage2(string Outstage2, string in) {
    _out2.open(Outstage2);
    _out2 << fixed << setprecision(7);
    ifstream _in2(in);
    if(!_in2.is_open()) {
        cout << "No such input file for stage 2.\n";
        exit(1);
    }
    string line;
    while(getline(_in2, line)) {
        if(line.empty()) {
            _out2 << "\n"; 
            continue; 
        }
        // cout << line << "\n"; 
        istringstream iss(line);
        vector< double > coords(3);
        for(int i = 0 ; i < 3; ++i) {
            iss >> coords[i];
        }
        coords.push_back(1.0); // Homogeneous coordinate
        coords = ViewTrans.apply(coords);
        for(int i =0 ;i < 3 ; ++i ) _out2 << coords[i] << " ";
        _out2 << "\n"; 
    }
    _in2.close();
    _out2.close(); 
    ProcessStage3(OutDir + "/stage3.txt", OutDir + "/stage2.txt");
    return; 
}


void Parser::computeViewTransMatrix() {
    //// cam[0] is the camera position, cam[1] is the look vector, cam[2] is the up vector
    Vect _l = (cam[1] - cam[0]).normalize(); 
    Vect _r =  (_l ^  cam[2]).normalize(); 
    Vect _u = (_r ^ _l); 
    _l = (_l * -1.0);
    Matrix Translation, Rotation;
    Translation.fillTranslation({-cam[0].x, -cam[0].y, -cam[0].z});
    Rotation.createRotation({_r, _u, _l});
    ViewTrans = Rotation * Translation;
    return; 
}


Parser ::Parser(string in, string out, string config) {
    OutDir = out; 
    _in.open(in);
    _out.open(out + "/stage1.txt");
    if(!_in.is_open()) {
        cout << "No such input file." << endl;
        exit(1);
    }
    if(!_out.is_open()) {
        cout << "No such output file." << endl;
        exit(1);
    }
    _stk.push(Matrix::getIdent());
    cam.resize(3);
    for(int i =0 ; i < 3; ++i) {
        _in >> cam[i];
    }
    perspect.resize(4);
    for(int i = 0 ; i < 4; ++i) {
        _in >> perspect[i];
    }
    computeViewTransMatrix(); 
    computeProjectionMatrix();
    ReadConfig(config);
    Initialize_Frame_Zbuffer();
}


void Parser::Initialize_Frame_Zbuffer() {
    Frame.resize(ScHeight, vector< vector< unsigned char >> (ScWidth, vector< unsigned char > (3, 0.0)));
    Zbuffer.resize(ScHeight, vector< double >(ScWidth, Z_rear_limit)); 
    return; 
}

void Parser::ReadConfig(string config) {
    ifstream inConfig(config);
    if(!inConfig.is_open()) {
        cout << "No such config file." << endl;
        exit(1);
    }
    inConfig >> ScWidth >> ScHeight;
    inConfig >> X_left_limit >> Y_bottom_limit >> Z_front_limit >> Z_rear_limit;
    dx = (-X_left_limit * 2.0) / ScWidth;
    dy = (-Y_bottom_limit * 2.0) / ScHeight;
    inConfig.close();
}

void Parser::_Translate(vector< double > amm) {
    Matrix T;
    T.Ident();
    for(int i = 0 ; i < 3; ++i) {
        T._matrix[i][3] = amm[i]; 
    }
    assert(_stk.size()> 0);
    // _stk.push(_stk.top() * T); 
    _stk.top() = _stk.top() * T;
    return; 
}


void Parser::_Rotate(double angle , vector < double > a) {
    Matrix R = Matrix::getIdent(); 
    Vect _a(a[0], a[1], a[2]);
    _a = _a.normalize();
    Vect i(1.0, 0.0, 0.0);
    Vect j(0.0, 1.0, 0.0);
    Vect k(0.0, 0.0, 1.0);
    vector< Vect > c(3);
    c[0] = i.rotate(angle, _a);
    c[1] = j.rotate(angle, _a);
    c[2] = k.rotate(angle, _a);
    for(int i = 0 ; i < 3; ++i) {
        R._matrix[0][i] = c[i].x;
        R._matrix[1][i] = c[i].y;
        R._matrix[2][i] = c[i].z;
    }
    assert(_stk.size() > 0);
    // _stk.push(_stk.top() * R);
    _stk.top() = _stk.top() * R;
    return; 
}


void Parser::_Scale(vector< double > scales) {
    Matrix S = Matrix::getIdent();
    for(int i = 0 ; i < 3; ++i ) {
        S._matrix[i][i] = scales[i]; 
    }
    assert(_stk.size() > 0);
    // _stk.push(_stk.top() * S);
    _stk.top() = _stk.top() * S;
    return;
}


void Parser::_Push() {
    assert(_stk.size() > 0);
    _stk.push(_stk.top());
    return;
}

void Parser::_Pop() {
    assert(_stk.size() > 1);
    _stk.pop();
    return;
}

vector< double > Parser::_applyTransformation(vector< double > coord) {
    assert(_stk.size() > 0);
    return _stk.top().apply(coord);
}

void Parser::_drawTriangle(vector< vector< double >> coords) {
    assert(_out.is_open());
    _out << fixed << setprecision(7); 
    for(int i = 0 ; i < 3; ++i) {
        _out << coords[i][0] << " " << coords[i][1] << " " << coords[i][2] << "\n";
    }
    _out << "\n"; 
    return;
}

void Parser::PrintCamAndPers() {
    cout << "Camera: " << endl;
    for(int i = 0 ; i < 3; ++i) {
        cout << cam[i].x << " " << cam[i].y << " " << cam[i].z << "\n";
    }
    cout << "Perspective: " << endl;
    for(int i = 0 ; i < 4; ++i) {
        cout << perspect[i] << " ";
    }
    cout << "\n";
}

void Parser::_Parse() {
    string cmd;
    while(_in >> cmd) {
        if(cmd == "translate") {
            vector< double > t(3);
            _in >> t[0] >> t[1] >> t[2];
            this->_Translate(t);
        }
        else if(cmd == "rotate") {
            double angle; 
            vector< double > a(3); 
            _in >> angle >> a[0] >> a[1] >> a[2];
            this->_Rotate(angle, a);  
        }
        else if(cmd == "scale") {
            vector< double > scales(3);
            _in >> scales[0] >> scales[1] >> scales[2];
            this->_Scale(scales);
        }
        else if(cmd == "push") {
            this->_Push();
        }
        else if(cmd == "pop") {
            this->_Pop();
        }
        else if(cmd == "triangle") {
            vector< vector< double >> coords(3, vector< double >(4));
            for(int i = 0 ; i < 3; ++i) {
                _in >> coords[i][0] >> coords[i][1] >> coords[i][2];
                coords[i][3] = 1.0;
                coords[i] = this->_applyTransformation(coords[i]);
            }
            this->_drawTriangle(coords);
        }
        else if(cmd == "end") {
            break;
        }
        else {
            cout << "Unknown command: " << cmd << endl;
            exit(1);
        }
    }
    _in.close();
    _out.close();
    ProcessStage2(OutDir + "/stage2.txt", OutDir + "/stage1.txt"); 
    return;
}
