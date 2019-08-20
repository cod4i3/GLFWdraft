#pragma once
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// ウィンドウ関連の処理
class Window{
    //ウィンドウのハンドル
    GLFWwindow *const window;
    
    //ウィンドウのサイズ
    GLfloat size[2];
    
    //ワールド座標系に対するデバイス座標系の拡大率
    GLfloat scale;
    
    //図形の正規化デバイス座標系上での位置
    GLfloat location[2];
    
    int keyStatus;
    
public:
    //コンストラクタ
    Window(int width = 640, int height = 480, const char *title = "Hello!")
    : window(glfwCreateWindow(width, height, title, NULL, NULL))
    , scale(100.0f),location{ 0.0f, 0.0f}, keyStatus(GLFW_RELEASE)
    {
        if(window == NULL)
        {
            //ウィンドウが作成できなかった
            std::cerr << "Can't create GLFW window." << std::endl;
            exit(1);
        }
        
        //現在のウィンドウを処理対象にする
        glfwMakeContextCurrent(window);
        
        //GLEWを初期化する
        glewExperimental = GL_TRUE;
        if(glewInit() != GLEW_OK)
        {
            //GLEWの初期化に失敗した
            std::cerr << "Can't initialize GLEW" << std::endl;
            exit(1);
        }
        
        // 垂直同期のタイミングを待つ
        glfwSwapInterval(1);
        
        //このインスタンスの this ポインタを記録しておく
        glfwSetWindowUserPointer(window, this);
        
        // マウスホイール操作時に呼び出す処理の登録
        glfwSetScrollCallback(window, wheel);
        
        // キーボード操作時に呼び出す処理の登録
        glfwSetKeyCallback(window, keyboard);
        
        // ウィンドウのサイズ変更時に呼び出す処理の登録
        glfwSetWindowSizeCallback(window,resize);
        
        //開いたウィンドウの初期設定
        resize(window,width,height);
    }
    
    //デストラクタ
    virtual ~Window(){
        glfwDestroyWindow(window);
    }
    
    //描画ループの継続判定
    explicit operator bool(){
        
        //イベントを取り出す（イベントが発生するまでプログラムを停止）
        glfwPollEvents();
                
                
        //キーボードの状態を調べる
        if(glfwGetKey(window, GLFW_KEY_LEFT) != GLFW_RELEASE)
            location[0] -= 10.0f / size[0];
        else if(glfwGetKey(window, GLFW_KEY_RIGHT) != GLFW_RELEASE)
            location[0] += 10.0f / size[0];
        
        if(glfwGetKey(window, GLFW_KEY_DOWN) != GLFW_RELEASE)
            location[1] -= 10.0f / size[1];
        else if(glfwGetKey(window, GLFW_KEY_UP) != GLFW_RELEASE)
            location[1] += 10.0f / size[1];
        
        
        //マウスの左ボタンの状態を調べる
        /*
         GLFW_MOUSE_BUTTON_1 と　GLFW_MOUSE_BUTTON_LEFTは左ボタン
         GLFW_MOUSE_BUTTON_2 と　GLFW_MOUSE_BUTTON_RIGHTは右ボタン
         GLFW_MOUSE_BUTTON_3 と　GLFW_MOUSE_BUTTON_MIDDLEは中ボタン
         GLFW_MOUSE_BUTTON_4 ~ GLFW_MOUSE_BUTTON_8 はマウス依存
         GLFW_MOUSE_BUTTON_LAST はGLFW_MOUSE_BUTTON_8と同じ
         */
        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) != GLFW_RELEASE)
        {
            //マウスの左ボタンが押されていたらマウスカーソルの位置を取得する
            double x,y;
            glfwGetCursorPos(window, &x, &y);
            
            //マウスカーソルの正規化デバイス座標系上での位置を求める
            location[0] = static_cast<GLfloat>(x) * 2.0f / size[0] - 1.0f;
            location[1] = 1.0f - static_cast<GLfloat>(y) * 2.0f / size[1];
        }
        
        
        return !glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE);
    }
    
    // ダブルバッファリング
    void swapBuffers(){
        //カラーバッファを入れ替える
        glfwSwapBuffers(window);
    }
    
    
    //ウィンドウのサイズを取り出す
    const GLfloat *getSize() const { return size; }
    
    //ワールド座標系に対するデバイス座標系の拡大率を取り出す
    GLfloat getScale() const{ return scale; }
    
    //位置を取り出す
    const GLfloat *getLocation() const { return location;}
    
    // ウィンドウのサイズ変更時の処理
    static void resize(GLFWwindow *const window,int width,int height)
    {
        //フレームバッファのサイズを調べる
        int fbWidth,fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        //ウィンドウ全体をビューポートに設定する
        glViewport(0, 0, fbWidth, fbHeight);
        
        //このインスタンスのthisポインタを得る
        Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));
        
        if(instance != NULL){
            //開いたウィンドウのサイズを保存する
            instance -> size[0] = static_cast<GLfloat>(width);
            instance -> size[1] = static_cast<GLfloat>(height);
        }
        
    }
    
    static void wheel(GLFWwindow *window, double x,double y)
    {
        //このインスタンスのthisポインタを得る
        Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));
        
        if(instance != NULL){
            //ワールド座標系に対するデバイス座標系の拡大率を更新する
            instance -> scale += static_cast<GLfloat>(y);
        }
    }
    
    static void keyboard(GLFWwindow *window, int key, int scancode,int action,int mods)
    {
        Window *const instance(static_cast<Window *>(glfwGetWindowUserPointer(window)));
        
        if(instance != NULL){
            //キーの状態を保存する
            instance -> keyStatus = action;
        }
    }
};
