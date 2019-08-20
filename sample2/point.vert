#version 150 core
uniform mat4 modelview;
uniform mat4 projection;
uniform mat3 normalMatrix;
const int Lcount = 2;
uniform vec4 Lpos[Lcount]; //光源の位置
uniform vec3 Lspec[Lcount]; //光源強度の鏡面反射光成分
layout (std140) uniform Material //材質データ
{
    vec3 Kamb;  // 環境光の反射係数
    vec3 Kdiff; //拡散反射係数
    vec3 Kspec; //鏡面反射係数
    float Kshi; //輝き係数
};
in vec4 position;
in vec3 normal;  //法線
out vec3 Idiff;  //拡散反射光強度
out vec3 Ispec;  //鏡面反射光強度
out vec4 P;
out vec3 N;

void main(){
    P = modelview * position; //頂点の位置
    N = normalize(normalMatrix * normal);  //鏡面に対する法線ベクトル
    vec3 V = -normalize(P.xyz);  //視点方向のベクトル
    for(int i = 0; i < Lcount; i++)
    {
        vec3 L = normalize((Lpos[i] * P.w - P * Lpos[i].w).xyz); //光源方向のベクトル
        vec3 H = normalize(L + V);    //中間ベクトル反射がHと一致するかどうかの確率でIspecが求められる
                                      //正反射方向(ベクトルI、法線ベクトルN)を引数にする
    }
    gl_Position = projection * P;
}
