#pragma once
#include<bits/stdc++.h>
#include "Matrix.h"
#include "vector.h"
#include "bitmap_image.hpp"
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
    mt19937 rng;

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
    void rasterizeTriangle(vector< vector < double >> &tri); 
    void rasterizeTriangle2(vector< vector < double >> &tri); 
    void ProcessImage_Zbuffer(ofstream &out);
    ~Parser();
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


void Parser::rasterizeTriangle2(vector< vector< double >> &tri) {
    struct P { double x, y, z ; };

    array< P, 3 > points; 
    for(int i = 0 ; i < 3; ++i ) { 
        points[i] = { tri[i][0], tri[i][1], tri[i][2] };
    }
    sort(points.begin(), points.end(), [](const P &a, const P &b) {
        return a.y >  b.y;
    });

    double topY = -Y_bottom_limit;
    double leftX = X_left_limit + dx/2;

    double top_scan = min(points[0].y, topY), bott_scan = max(points[2].y, Y_bottom_limit);
    
    int raw_top = int(round((top_scan - Y_bottom_limit) / dy));
    int raw_bottom = int(round((bott_scan - Y_bottom_limit) / dy));

    int top_row = min(raw_top, ScHeight - 1);
    int bottom_row = max(raw_bottom, 0);

    uniform_int_distribution<int> dist(0, 255);
    vector< unsigned char > color = { (unsigned char)dist(rng), (unsigned char )dist(rng), (unsigned char)dist(rng)};

    for(int row = top_row; row >= bottom_row; --row) { 
        double y_s = Y_bottom_limit + row * dy + dy / 2.0;

        vector< pair< double, double >> intersections;
        auto Inter = [&](const P &p1, const P &p2) {
            if(y_s >= min(p1.y, p2.y) && y_s <= max(p1.y, p2.y) && fabs(p1.y - p2.y) > 1e-9) {
                double t = (y_s - p1.y) / (p2.y - p1.y);
                double x_i = p1.x + t * (p2.x - p1.x);
                double z_i = p1.z + t * (p2.z - p1.z);
             intersections.push_back({x_i, z_i});
                return; 
            }
        }; 
        Inter(points[0], points[1]);
        Inter(points[1], points[2]);
        Inter(points[2], points[0]);
        if( intersections.size() < 2) continue;
        if(intersections[0].first > intersections[1].first) {
            swap(intersections[0], intersections[1]);
        }

        int c_l = clamp(int(round((intersections[0].first - leftX) / dx)), 0, ScWidth - 1);
        int c_r = clamp(int(round((intersections[1].first - leftX) / dx)), 0, ScWidth - 1);
        if(c_l > c_r) continue;

        for(int col = c_l; col <= c_r ; ++col) {
            double x_p = leftX + col * dx;
            double z_p = intersections[0].second + (x_p - intersections[0].first) * 
                         (intersections[1].second - intersections[0].second) / (intersections[1].first - intersections[0].first);
            int _row = ScHeight - 1 - row; 

            if(z_p < Zbuffer[_row][col]) {
                Zbuffer[_row][col] = z_p; 
                Frame[_row][col] = color;
            }
        }
    }

    return;  
}


void Parser::rasterizeTriangle(vector< vector< double >> &tri) {
    vector< double > vx(3), vy(3), vz(3); 

    for(int i = 0; i < 3; ++i) {
        vx[i] = tri[i][0], vy[i] = tri[i][1];
        vz[i] = tri[i][2]; 
    }

    double Y_top = -Y_bottom_limit;
    double X_left = X_left_limit + dx/2;
    int top_row = (int)((Y_top - *max_element(vy.begin(), vy.end())) / dy);
    int bottom_row = (int)((Y_top - *min_element(vy.begin(), vy.end())) / dy);

    top_row = clamp(top_row, 0, ScHeight - 1);
    bottom_row = clamp(bottom_row, 0, ScHeight - 1);

    
    uniform_int_distribution<int> dist(0, 255);
    vector< unsigned char > color = { (unsigned char)dist(rng), (unsigned char )dist(rng), (unsigned char)dist(rng)};

    for(int row = top_row; row <= bottom_row; ++row) {
        double y_c = Y_top - row*dy - dy/2;
        vector< pair< double, double >> I(2); 
        int cnt = 0;
        for(auto [i,j] : vector< pair< int, int>>{{0, 1}, {1, 2}, {2, 0}}) {
            if((vy[i] <= y_c && vy[j] >= y_c) || (vy[i] >= y_c && vy[j] <= y_c)) {
                if(vy[i] == vy[j]) continue; ////horizontal edge. So skip it.
                double t = (y_c - vy[i]) / (vy[j] - vy[i]);
                double x_i = vx[i] + t * (vx[j] - vx[i]);
                double z_i = vz[i] + t * (vz[j] - vz[i]);
                if(cnt  < 2) I[cnt++] = {x_i, z_i};
            }
        }
        if(cnt < 2) continue; 
        if(I[0].first > I[1].first) {
            swap(I[0], I[1]); 
        }

        double x_l = I[0].first, x_r = I[1].first;
        double z_l = I[0].second, z_r = I[1].second;
        int c_l = clamp( int( ceil((x_l - X_left)/dx) ), 0, ScWidth-1 );
        int c_r = clamp( int( floor((x_r - X_left)/dx) ), 0, ScWidth-1 );
        if (c_l > c_r) continue;  
        double span = c_r - c_l; 
        double dz = (z_r - z_l)/ span; 
        double zcur = z_l + ( (X_left + c_l*dx) < x_l ? dz*0.5 : 0.0 );


        for(int col = c_l ; col <= c_r ; ++col) {
            if(zcur < Zbuffer[row][col]) {
                Zbuffer[row][col] = zcur; 
                Frame[row][col] = color;
            }
            zcur += dz; 
        }
        return; 
    }
}


void Parser::ProcessStage4(string Outstage4, string in) {
    _out4.open(Outstage4);
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
                rasterizeTriangle2(tri);
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
    ProcessImage_Zbuffer(_out4);
    _out4.close(); 
    tri.clear();
    return;
}

void Parser::ProcessImage_Zbuffer(ofstream &out) {
    bitmap_image image(ScWidth, ScHeight);
    for(int y = 0; y < ScHeight; ++y) {
        for(int x = 0 ; x < ScWidth; ++x) {
            auto &color = Frame[y][x];
            image.set_pixel(x, y, color[0], color[1], color[2]);
        }
    }
    image.save_image(OutDir + "/out.bmp");
    ////now for the z-buffer.
    out << fixed << setprecision(6);
    for(int y = 0; y < ScHeight; ++y) {
        for(int x = 0 ; x < ScWidth; ++x) {
            if(Zbuffer[y][x] < Z_rear_limit) {
                out << Zbuffer[y][x] << "\t";
            }
        } 
        out << "\n";
    }
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
    rng = mt19937(12345);
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


Parser::~Parser() {
    Frame.clear();
    Zbuffer.clear();
    Frame.shrink_to_fit();
    Zbuffer.shrink_to_fit();
}