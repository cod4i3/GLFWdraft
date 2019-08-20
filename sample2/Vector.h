#pragma once
#include <array>

//変換強烈
#include "Matrix.h"

//ベクトル
using Vector = std::array<GLfloat, 4>;

// 行列とベクトルの乗算
//  m: Matrix型の行列
//  v: Vector型のベクトル
Vector operator*(const Matrix &m, const Vector &v)
{
    Vector t;
    
    for(int i = 0; i < 4; i++)
    {
        t[i] =
            m.data()[0 + i] * v[0] +
            m.data()[4 + i] * v[1] +
            m.data()[8 + i] * v[2] +
            m.data()[12 + i]* v[3];
    }
    
    return t;
}
