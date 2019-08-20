#pragma once
#include <cmath>
#include <algorithm>
#include <GL/glew.h>

// 変換行列
class Matrix
{
    //変換行列の要素(3次元に対する同次座標）
    GLfloat matrix[16];
    
public:
    // コンストラクタ
    Matrix() {}
    
    // 配列の内容で初期化するコンストラクタ
    // a: GLfloat型の 16 要素の行列
    Matrix(const GLfloat *a)
    {
        std::copy(a, a+16, matrix);
    }
    
    //変換行列の配列を返す
    const GLfloat *data() const
    {
        return matrix;
    }
    
    //法線ベクトルの変換行列を求める
    void getNormalMatrix(GLfloat *m) const
    {
        m[0] = matrix[ 5] * matrix[10] - matrix[ 6] * matrix[ 9];
        m[1] = matrix[ 6] * matrix[ 8] - matrix[ 4] * matrix[10];
        m[2] = matrix[ 4] * matrix[ 9] - matrix[ 5] * matrix[ 8];
        m[3] = matrix[ 9] * matrix[ 2] - matrix[10] * matrix[ 1];
        m[4] = matrix[10] * matrix[ 0] - matrix[ 8] * matrix[ 2];
        m[5] = matrix[ 8] * matrix[ 1] - matrix[ 9] * matrix[ 0];
        m[6] = matrix[ 1] * matrix[ 6] - matrix[ 2] * matrix[ 5];
        m[7] = matrix[ 2] * matrix[ 4] - matrix[ 0] * matrix[ 6];
        m[8] = matrix[ 0] * matrix[ 5] - matrix[ 1] * matrix[ 4];
    }
    
    
    //単位行列を設定する
    void loadIdentity()
    {
        std::fill(matrix, matrix+16, 0.0f);
        matrix[ 0] = matrix[ 5] = matrix[10] = matrix[15] = 1.0f;
    }
    
    //単位行列を作成する
    static Matrix identity()
    {
        Matrix t;
        t.loadIdentity();
        return t;
    }
    
    // (x,y,z)だけ平行移動する変換行列を作成する
    static Matrix translate(GLfloat x,GLfloat y, GLfloat z)
    {
        Matrix t;
        
        // 4*4の単位行列、一番右の列上から3つが(t_x,t_y,t_z)
        // これに(x,y,z,1)^T をかけると(x+t_x , y+t_y z+t_z , 1) となり平行移動
        t.loadIdentity();
        t.matrix[12] = x;
        t.matrix[13] = y;
        t.matrix[14] = z;
        
        return t;
    }
    
    // 単位行列の(0,0),(1,1),(2,2)が(s_x,s_y,s_z)で拡大行列
    static Matrix scale(GLfloat x, GLfloat y, GLfloat z)
    {
        Matrix t;
        
        t.loadIdentity();
        t.matrix[ 0] = x;
        t.matrix[ 5] = y;
        t.matrix[10] = z;
        
        return t;
    }
    
    //(x,y,z)を軸にa回転する行列を作成する
    static Matrix rotate(GLfloat a, GLfloat x, GLfloat y, GLfloat z)
    {
        Matrix t;
        const GLfloat d(sqrt(x * x + y * y + z * z));
        
        if(d > 0.0f)
        {
            const GLfloat l(x / d), m(y / d), n(z / d);
            const GLfloat l2(l * l), m2(m * m), n2(n * n);
            const GLfloat lm(l * m), mn(m * n), nl(n * l);
            const GLfloat c(cos(a)), c1(1.0f - c), s(sin(a));
            
            t.loadIdentity();
            t.matrix[ 0] = (1.0f - l2) * c + l2;
            t.matrix[ 1] = lm * c1 + n * s;
            t.matrix[ 2] = nl * c1 - m * s;
            t.matrix[ 4] = lm * c1 - n * s;
            t.matrix[ 5] = (1.0f - m2) * c + m2;
            t.matrix[ 6] = mn * c1 + l * s;
            t.matrix[ 8] = nl * c1 + m * s;
            t.matrix[ 9] = mn * c1 - l * s;
            t.matrix[10] = (1.0f - n2) * c + n2;
        }
        return t;
    }
    
    //行列の乗算
    Matrix operator* (const Matrix &m) const
    {
        Matrix t;
        
        for(int i = 0; i < 16; i++){
            const int j(i & 3), k(i & ~3);
            
            t.matrix[i] =
            matrix[ 0 + j] * m.matrix[k + 0] +
            matrix[ 4 + j] * m.matrix[k + 1] +
            matrix[ 8 + j] * m.matrix[k + 2] +
            matrix[12 + j] * m.matrix[k + 3];
        }
        return t;
    }
    
    //ビュー変換行列を作成する
    static Matrix lookat(
    GLfloat ex, GLfloat ey, GLfloat ez,   //視点の位置
    GLfloat gx, GLfloat gy, GLfloat gz,   //目標点の位置
    GLfloat ux, GLfloat uy, GLfloat uz)   //上方向のベクトル
    {
        // 平行移動の変換行列
        const Matrix tv(translate(-ex, -ey, -ez));
        
        // t軸 = e - g
        const GLfloat tx(ex - gx);
        const GLfloat ty(ey - gy);
        const GLfloat tz(ez - gz);
        // t(1,1,1) r(1,0,-1) s(-1,2,-1)
        // r軸 = u軸 × t 軸
        const GLfloat rx(uy * tz - uz * ty);
        const GLfloat ry(uz * tx - ux * tz);
        const GLfloat rz(ux * ty - uy * tx);
        
        // s軸 = t 軸　× r 軸
        const GLfloat sx(ty * rz - tz * ry);
        const GLfloat sy(tz * rx - tx * rz);
        const GLfloat sz(tx * ry - ty * rx);
        
        // s軸の長さのチェック
        const GLfloat s2(sx * sx + sy * sy + sz * sz);
        if(s2 == 0.0f) return tv;
        
        // 回転の変換行列
        Matrix rv;
        rv.loadIdentity();
        
        // r軸を正規化して配列変数に格納
        const GLfloat r(sqrt(rx * rx + ry * ry + rz * rz));
        rv.matrix[ 0] = rx / r;
        rv.matrix[ 4] = ry / r;
        rv.matrix[ 8] = rz / r;
        
        // s軸を正規化して配列変数に格納
        const GLfloat s(sqrt(s2));
        rv.matrix[ 1] = sx / s;
        rv.matrix[ 5] = sy / s;
        rv.matrix[ 9] = sz / s;
        
        // t軸を正規化して配列変数に格納
        const GLfloat t(sqrt(tx * tx + ty * ty + tz * tz));
        rv.matrix[ 2] = tx / t;
        rv.matrix[ 6] = ty / t;
        rv.matrix[10] = tz / t;
        
        // 視点の平行移動の変換行列に視線の回転の変換行列を乗じる
        return rv * tv;
        
    }
    
    // 直交投影変換行列を作成する
    static Matrix orthogonal(GLfloat left, GLfloat right,
                             GLfloat bottom, GLfloat top,
                             GLfloat zNear, GLfloat zFar)
    {
        Matrix t;
        const GLfloat dx(right - left);
        const GLfloat dy(top - bottom);
        const GLfloat dz(zFar - zNear);
        
        if( dx != 0.0f && dy != 0.0f && dz != 0.0f)
        {
            t.loadIdentity();
            t.matrix[ 0] = 2.0f / dx;
            t.matrix[ 5] = 2.0f / dy;
            t.matrix[10] = -2.0f / dz;
            t.matrix[12] = -(right + left) /dx;
            t.matrix[13] = -(top + bottom) / dy;
            t.matrix[14] = -(zFar + zNear) / dz;
        }
        return t;
    }
    
    // 透視投影変換行列を作成する
    static Matrix frustum(GLfloat left, GLfloat right,
                          GLfloat bottom, GLfloat top,
                          GLfloat zNear, GLfloat zFar)
    {
        Matrix t;
        const GLfloat dx(right - left);
        const GLfloat dy(top - bottom);
        const GLfloat dz(zFar - zNear);
        
        if(dx != 0.0f && dy != 0.0f && dz != 0.0f)
        {
            t.loadIdentity();
            t.matrix[ 0] = 2.0f * zNear / dx;
            t.matrix[ 5] = 2.0f * zNear / dy;
            t.matrix[ 8] = (right + left) / dx;
            t.matrix[ 9] = (top + bottom) / dy;
            t.matrix[10] = -(zFar + zNear) / dz;
            t.matrix[11] = -1.0f;
            t.matrix[14] = -2.0f * zFar * zNear / dz;
            t.matrix[15] = 0.0f;
        }
        
        return t;
    }
    
    // 画角を指定して透視投影変換行列を作成する
    static Matrix perspective(GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar)
    {
        Matrix t;
        const GLfloat dz(zFar - zNear);
        
        if(dz != 0.0f)
        {
            t.loadIdentity();
            t.matrix[ 5] = 1.0f / tan(fovy * 0.5f);
            t.matrix[ 0] = t.matrix[5] / aspect;
            t.matrix[10] = -(zFar + zNear) / dz;
            t.matrix[11] = -1.0f;
            t.matrix[14] = -2.0f * zFar * zNear / dz;
            t.matrix[15] = 0.0f;
        }
        
        return t;
    }
    
   
};
