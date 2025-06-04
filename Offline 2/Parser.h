#pragma once
#include<bits/stdc++.h>
#include "Matrix.h"
#include "vector.h"
#define PI acos(-1.0)
using namespace std;

class Parser {
    public:
    vector< Vect > cam; 
    vector< double > perspect; 
    ifstream _in; 
    ofstream _out, _out2;
    stack< Matrix > _stk; 
    Matrix ViewTrans;
    string OutDir; 

    Parser(string in, string out); 
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
    void ProcessStage2(string Outstage2, string in); 
};


void Parser::ProcessStage2(string Outstage2, string in) {
    _out2.open(Outstage2);
    if(!_out2.is_open()) {
        cout << "No such output file for stage 2.\n";
        exit(1);
    }
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


Parser ::Parser(string in, string out) {
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
