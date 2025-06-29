#pragma once
#include<bits/stdc++.h>
#include "vector.h"
using namespace std;


class Matrix {
    public:
    vector<vector< double >> _matrix;
    Matrix(double val = 0.0)
    {
        _matrix.resize(4, vector< double >(4,val));
    }
    vector< double > apply(vector< double > coord);
    void Ident();
    static Matrix getIdent(); 
    Matrix operator*(Matrix const & B) const; 
    void fillTranslation(vector< double > trans);
    void createRotation(vector< Vect > allAxis);
};

void Matrix::createRotation(vector< Vect > allAxis) {
    Ident();
    for(int i = 0 ; i < 3; ++i) {
        _matrix[i][0] = allAxis[i].x;
        _matrix[i][1] = allAxis[i].y;
        _matrix[i][2] = allAxis[i].z;
    }
}

void Matrix::fillTranslation(vector< double > trans) {
    Ident();
    for(int i = 0 ; i < 3; ++i) {
        _matrix[i][3] = trans[i];
    }
    return;
}

vector< double > Matrix::apply(vector< double > coord)
{
    vector< double > result(4, 0.0);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result[i] += _matrix[i][j] * coord[j];
        }
    }
    return result;
}


void Matrix::Ident() {
    for(int i = 0; i < 4; ++i ) {
        for(int j = 0; j < 4; ++j) {
            this->_matrix[i][j] = (i == j ? 1.0: 0.0); 
        }
    }
    return; 
}

Matrix Matrix::getIdent() {
    Matrix I;
    I.Ident();
    return I; 
}

Matrix Matrix::operator*(Matrix const & B) const  {
    Matrix C(0.0);
    for(int i = 0; i < 4; ++i) {
        for(int j = 0; j < 4; ++j) {
            for(int k = 0 ;k < 4; ++k) {
                C._matrix[i][j] += this->_matrix[i][k] * B._matrix[k][j]; 
            }
        }
    }
    return C;  
}  