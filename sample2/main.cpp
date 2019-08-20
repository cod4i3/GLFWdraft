#include<cstdlib>
#include<cmath>
#include<iostream>
#include<fstream>
#include<vector>
#include<memory>
#include<GL/glew.h>
#include<GLFW/glfw3.h>
#include "Window.h"
#include "Matrix.h"
#include "Vector.h"
#include "Shape.h"
#include "ShapeIndex.h"
#include "SolidShapeIndex.h"
#include "SolidShape.h"
#include "Uniform.h"
#include "Material.h"
/*
 Vectorはベクトルであってvectorではない
 紛らわしいがこれが一番名前として分かりやすいので採用
*/

// シェーダオブジェクトのコンパイル結果を表示する
// shader: シェーダオブジェクト名
// str:    コンパイルエラーが発生した場所を示す文字列

GLboolean printShaderInfoLog(GLuint shader,const char *str){
    //コンパイル結果を取得する
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(status == GL_FALSE) std :: cerr << "Compile Error in " << str << std::endl;
    
    //シェーダのコンパイル時のログの長さを取得する
    GLsizei bufSize;
    glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&bufSize);
    
    if(bufSize > 1){
        //シェーダのコンパイル時のログの内容を取得する
        std::vector<GLchar> infoLog(bufSize);
        GLsizei length;
        glGetShaderInfoLog(shader,bufSize,&length,&infoLog[0]);
        std::cerr << &infoLog[0] << std::endl;
    }
    
    return static_cast<GLboolean>(status);
}

//プログラムオブジェクトのリンク結果をg表示する
// program:プログラムオブジェクト名

GLboolean printProgramInfoLog(GLuint program){
    //リンク結果を取得する
    GLint status;
    glGetProgramiv(program,GL_LINK_STATUS,&status);
    if(status == GL_FALSE) std::cerr << "Link Error." << std::endl;
    
    //シェーダのリンク時のログの長さを取得する
    GLsizei bufsize;
    glGetProgramiv(program,GL_INFO_LOG_LENGTH,&bufsize);
    
    if(bufsize > 1){
        //シェーダのリンク時のログの内容を取得する
        std::vector<GLchar> infoLog(bufsize);
        GLsizei length;
        glGetProgramInfoLog(program,bufsize,&length,&infoLog[0]);
        std::cerr << &infoLog[0] << std::endl;
    }
    
    return static_cast<GLboolean>(status);
}


//   プログラムオブジェクトを作成する
//   vsrc: バーテックスシェーダのソースプログラムの表示
//   fsrc: フラグメントシェーダのソースプログラムの文字列

GLuint createProgram(const char *vsrc, const char *fsrc){
    //空のプログラムオブジェクトを作成する
    const GLuint program(glCreateProgram());
    
    //バーテックスシェーダのソースがあった時
    if(vsrc != NULL){
        //バーテックスシェーダのシェーダオブジェクトを作成する
        const GLuint vobj(glCreateShader(GL_VERTEX_SHADER));
        glShaderSource(vobj,1,&vsrc,NULL);
        glCompileShader(vobj);
        
        if(printShaderInfoLog(vobj,"vertex shader"))
            glAttachShader(program,vobj);
        glDeleteShader(vobj);
        
        
    }
    
    
    if(fsrc != NULL){
        //フラグメントシェーダのシェーダオブジェクトを作成する
        const GLuint fobj(glCreateShader(GL_FRAGMENT_SHADER));
        glShaderSource(fobj,1,&fsrc,NULL);
        glCompileShader(fobj);
        
        //フラグメントシェーダのシェーダオブジェクトをプログラムオブジェクトに組み込む
        if(printShaderInfoLog(fobj,"fragment shader"))
            glAttachShader(program,fobj);
        glDeleteShader(fobj);
    }
    
    //プログラムオブジェクトをリンクする
    glBindAttribLocation(program,0,"position");
    glBindAttribLocation(program, 1, "normal");
    glBindFragDataLocation(program,0,"fragment");
    glLinkProgram(program);
    
    //作成したプログラムオブジェクトを返す
    if(printProgramInfoLog(program))
        return program;
    
    //プログラムオブジェクトが作成できなければ0を返す
    glDeleteProgram(program);
    return 0;
}



//シェーダのソースファイルを読み込んだメモリを返す
//name: シェーダのソースファイル名
//buffer: 読み込んだソースファイルのテキスト
bool readShaderSource(const char *name,std::vector<GLchar> &buffer){
    //ファイル名がNULL
    if(name == NULL) return false;
    
    //ソースファイルを開く
    std::ifstream file(name,std::ios::binary);
    if(file.fail()){
        //開けなかった場合
        std::cerr << "Error: Can't open source file: " << name << std::endl;
        return false;
    }
    
    //ファイルの末尾に移動し現在位置（＝ファイルサイズ）を得る
    file.seekg(0L,std::ios::end);
    GLsizei length = static_cast<GLsizei>(file.tellg());
    
    //ファイルサイズのメモリを確保
    buffer.resize(length + 4);
    
    //ファイルを先頭から読み込む
    file.seekg(0L,std::ios::beg);
    file.read(buffer.data(),length);
    buffer[length] = '\n';
    
    if(file.fail()){
        //上手く読み込めなかった
        std::cerr << "Error: Could not read source file" << name << std::endl;
        file.close();
        return false;
    }
    
    //読み込み成功
    file.close();
    return true;
}


//シェーダのソースファイルを読み込んでプログラムオブジェクトを作成する
//  vert:バーテックスシェーダのソースファイル名
//  frag:フラグメントシェーダのソースファイル名
GLuint loadProgram(const char *vert,const char *frag){
    //シェーダのソースファイルを読み込む
    std::vector<GLchar> vsrc;
    const bool vstat(readShaderSource(vert, vsrc));
    std::vector<GLchar> fsrc;
    const bool fstat(readShaderSource(frag, fsrc));
    
    //プログラムオブジェクトを作成する
    return vstat && fstat ? createProgram(vsrc.data(), fsrc.data()) : 0;
}



// 面ごとに法線を変えた六面体の頂点属性
constexpr Object::Vertex solidCubeVertex[] =
{
    // 左
    { -1.0f, -1.0f, -1.0f,  -1.0f, 0.0f, 0.0f }, 
    { -1.0f, -1.0f,  1.0f,  -1.0f, 0.0f, 0.0f },
    { -1.0f,  1.0f,  1.0f,  -1.0f, 0.0f, 0.0f },
    { -1.0f, -1.0f, -1.0f,  -1.0f, 0.0f, 0.0f },
    { -1.0f,  1.0f,  1.0f,  -1.0f, 0.0f, 0.0f },
    { -1.0f,  1.0f, -1.0f,  -1.0f, 0.0f, 0.0f },
    // 裏
    {  1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f },
    { -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f },
    { -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f },
    {  1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f },
    { -1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f },
    {  1.0f,  1.0f, -1.0f, 0.0f, 0.0f, -1.0f },
    // 下
    { -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f },
    {  1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f },
    {  1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f },
    { -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f },
    {  1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f },
    { -1.0f, -1.0f,  1.0f, 0.0f, -1.0f, 0.0f },
    // 右
    { 1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f },
    { 1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f },
    { 1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f },
    { 1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 0.0f },
    { 1.0f,  1.0f, -1.0f, 1.0f, 0.0f, 0.0f },
    { 1.0f,  1.0f,  1.0f, 1.0f, 0.0f, 0.0f },
    // 上
    { -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f },
    { -1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
    {  1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
    { -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f },
    {  1.0f, 1.0f,  1.0f, 0.0f, 1.0f, 0.0f },
    {  1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f },
    // 前
    { -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
    {  1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
    {  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
    { -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
    {  1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f },
    { -1.0f,  1.0f, 1.0f, 0.0f, 0.0f, 1.0f }
};

// 六面体の面を塗りつぶす三角形の頂点のインデックス
constexpr GLuint solidCubeIndex[] =
{
    0, 1, 2, 3, 4, 5, // 左
    6, 7, 8, 9, 10, 11,//裏
    12, 13, 14, 15, 16, 17, // 下
    18, 19, 20, 21, 22, 23, // 右
    24, 25, 26, 27, 28, 29, // 上
    30, 31, 32, 33, 34, 35 //前
};

int main() {
    
    //GLFW初期化
    if(glfwInit() == GL_FALSE){
        std::cerr << "Can't initialize GLFW" << std::endl;
        return 1;
    }
    
    
    //プログラム終了時の処理を登録する
    atexit(glfwTerminate);
    
    // OpenGL Version 3.2 Core Profile を選択する
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER,GL_TRUE);
    
    //ウィンドウを作成する
    Window window;
    
    // 背景色を指定する
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    
    // 背面カリングを有効にする（デフォルトなら一番下だけでもOK)
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    
    // デプスバッファを有効にする (デフォルトなら一番下だけでもOK)
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    
    //プログラムオブジェクトを作成する
    const GLuint program(loadProgram("point.vert","point.frag"));
    
    // uniform変数の場所を取得する(2つ目のに指定したuniform変数の場所を探す）
    const GLint modelviewLoc(glGetUniformLocation(program, "modelview"));
    const GLint projectionLoc(glGetUniformLocation(program,"projection"));
    const GLint normalMatrixLoc(glGetUniformLocation(program, "normalMatrix"));
    const GLint LposLoc(glGetUniformLocation(program, "Lpos"));
    const GLint LambLoc(glGetUniformLocation(program, "Lamb"));
    const GLint LdiffLoc(glGetUniformLocation(program, "Ldiff"));
    const GLint LspecLoc(glGetUniformLocation(program, "Lspec"));
    
    // uniform blockの場所を取得する
    const GLint materialLoc(glGetUniformBlockIndex(program, "Material"));
    
    // uniform blockの場所を0番の結合ポイントに結びつける
    glUniformBlockBinding(program, materialLoc, 0);
    
    //球の分割数
    const int slices(16), stacks(8);
    
    //頂点属性を作る
    std::vector<Object::Vertex> solidSphereVertex;
    
    for(int j = 0; j <= stacks ; j++)
    {
        const float t(static_cast<float>(j) / static_cast<float>(stacks));
        const float y(cos(3.141693f * t)), r(sin(3.141693f * t));
        
        for(int i = 0; i <= slices; i++)
        {
            const float s(static_cast<float>(i) / static_cast<float>(slices));
            const float z(r * cos(6.283185f * s)), x(r * sin(6.283185f * s));
            
            //頂点属性
            const Object::Vertex v = { x, y, z, x, y, z};
            
            //頂点属性を追加する
            solidSphereVertex.emplace_back(v);
        }
    }
    
    //インデックスを作る
    std::vector<GLuint> solidSphereIndex;
    
    for(int j = 0; j < stacks; j++)
    {
        const int k((slices + 1) * j);
        
        for(int i = 0; i < slices; i++)
        {
            //頂点のインデックス (左上、右上、左下、右下)
            const GLuint k0(k + i);
            const GLuint k1(k0 + 1);
            const GLuint k2(k1 + slices);
            const GLuint k3(k2 + 1);
            
            //左下の三角形
            solidSphereIndex.emplace_back(k0);
            solidSphereIndex.emplace_back(k2);
            solidSphereIndex.emplace_back(k3);
            
            //右上の三角形
            solidSphereIndex.emplace_back(k0);
            solidSphereIndex.emplace_back(k3);
            solidSphereIndex.emplace_back(k1);
        }
    }

    // 図形データを作成する
    std::unique_ptr<const Shape> shape(new SolidShapeIndex(3,
        static_cast<GLsizei>(solidSphereVertex.size()),solidSphereVertex.data(),
        static_cast<GLsizei>(solidSphereIndex.size()),solidSphereIndex.data()));
    
    // 光源データを作成する（光の色は赤と白, 下二つの引数はRGB）
    static constexpr int Lcount(2);
    static constexpr Vector Lpos[] = {0.0f, 0.0f, 5.0f, 1.0f,  8.0f, 0.0f, 0.0f, 1.0f};//位置
    static constexpr GLfloat Lamb[] = {0.2f, 0.1f, 0.1f,  0.1f, 0.1f, 0.1f};  //環境光
    static constexpr GLfloat Ldiff[] = {1.0f, 0.5f, 0.5f,  0.9f, 0.9f, 0.9f}; //拡散反射光
    static constexpr GLfloat Lspec[] = {1.0f, 0.5f, 0.5f,  0.9f, 0.9f, 0.9f}; //鏡面反射光
    
    //色データ
    static constexpr Material color[] =
    {
        //     Kamb              Kdiff               Kspec        Kshi
        {0.6f, 0.6f, 0.2f,  0.6f, 0.6f, 0.2f,  0.3f, 0.3f, 0.3f,  30.0f},
        {0.1f, 0.1f, 0.5f,  0.1f, 0.1f, 0.5f,  0.4f, 0.4f, 0.4f,  60.0f}
    };
    
    const Uniform<Material> material(color, 2);
    
    // タイマーを0にセット
    glfwSetTime(0.0);
    
    //ウィンドウが開いている間繰り返す
    while(window)
    {
        // ウィンドウを消去
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // シェーダプログラムの使用開始(1つだけならwhileの前でも可能）
        glUseProgram(program);
        
        // 透視投影変換行列を求める
        const GLfloat *const size(window.getSize());
        const GLfloat fovy(window.getScale() * 0.01f);
        const GLfloat aspect(size[0] / size[1]);
        const Matrix projection(Matrix::perspective(fovy, aspect, 1.0f, 10.0f));
        
        // モデル変換行列を求める
        const GLfloat *const location(window.getLocation());
        const Matrix r(Matrix::rotate(static_cast<GLfloat>(glfwGetTime()),
                                      0.0f, 1.0f, 0.0f));
        const Matrix model(Matrix::translate(location[0], location[1], 0.0f) * r);
        
        // ビュー変換行列を求める
        const Matrix view(Matrix::lookat(3.0f, 4.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f));
        
        //法線ベクトルの変換行列の格納先
        GLfloat normalMatrix[9];
        
        //モデルビュー変換行列を求める
        const Matrix modelview(view * model);
        
        //法線ベクトルの変換行列を求める
        modelview.getNormalMatrix(normalMatrix);
         
        // uniform変数に値を設定する
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection.data());
        glUniformMatrix4fv(modelviewLoc, 1, GL_FALSE, modelview.data());
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, normalMatrix);
        for(int i = 0; i< Lcount; i++)
            glUniform4fv(LposLoc + i, 1, (view * Lpos[i]).data());
        glUniform3fv(LambLoc, Lcount, Lamb);
        glUniform3fv(LdiffLoc, Lcount, Ldiff);
        glUniform3fv(LspecLoc, Lcount, Lspec);
        
        //図形を描画する
        material.select(0, 0);
        shape -> draw();
        
        //二つ目のモデルビュー変換行列を求める
        const Matrix modelview1(modelview * Matrix::translate(0.0f, 0.0f, 3.0f));
        
        //二つ目の法線ベクトルの変換行列を求める
        modelview1.getNormalMatrix(normalMatrix);
        
        //uniform変数に値を設定する
        glUniformMatrix4fv(modelviewLoc, 1, GL_FALSE, modelview1.data());
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, normalMatrix);
        
        //二つ目の図形を描画する
        material.select(0, 1);
        shape -> draw();
        
        //カラーバッファを入れ替えてイベントを取り出す
        window.swapBuffers();
    }
}
