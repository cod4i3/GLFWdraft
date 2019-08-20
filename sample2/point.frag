#version 150 core
const int Lcount = 2;
uniform vec4 Lpos[Lcount]; //光源の位置
uniform vec3 Lamb[Lcount];
uniform vec3 Ldiff[Lcount];
uniform vec3 Lspec[Lcount]; //光源強度の鏡面反射光成分
layout (std140) uniform Material //材質データ
{
    vec3 Kamb;  // 環境光の反射係数
    vec3 Kdiff; //拡散反射係数
    vec3 Kspec; //鏡面反射係数
    float Kshi; //輝き係数
};
in vec3 Idiff;  //拡散反射光強度

in vec4 P;
in vec3 N;
out vec4 fragment;

void main(){
    vec3 V = -normalize(P.xyz);
    vec3 Idiff = vec3(0.0);
    vec3 Ispec = vec3(0.0);
    for(int i = 0; i < Lcount; i++)
    {
        vec3 L = normalize((Lpos[i] * P.w - P * Lpos[i].w).xyz);
        vec3 Iamb = Kamb * Lamb[i];
        Idiff += max(dot(N,L), 0.0) * Kdiff * Ldiff[i] + Iamb;
        vec3 H = normalize(L + V);
        
        Ispec += pow(max(dot(N,H), 0.0), Kshi) * Kspec * Lspec[i]; //反転してもしなくてもここで結果は同じになる
    }
    fragment = vec4(Idiff + Ispec, 1.0);
}
