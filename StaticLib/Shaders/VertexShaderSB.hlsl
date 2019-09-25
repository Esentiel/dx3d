#include "ShaderDefines.inc"

cbuffer MVPbuffer : register(b0)
{
    float4x4 mvp;
    float4x4 world;
    float4x4 viewProj;
    float4x4 shadowMapMatrix;
    float diffuseIntensity;
    float emissiveK;
    float ambientK;
    float roughness;
}

cbuffer PerScene : register(b1)
{
    float3 eyePos;
    LightSource lightS;
}

struct VS_IN
{
    float3 position : POSITION;
};

struct VS_OUT
{
    float3 posL : TEXCOORD;
    float4 projPos : SV_POSITION;
};

VS_OUT main(VS_IN input)
{
    VS_OUT output;

    output.posL = input.position;

    float4 posW = mul(float4(input.position, 1.f), world);

    // Always center sky about camera.
    posW.xyz += eyePos;

    // Set z = w so that z/w = 1 (i.e., skydome always on far plane).
    output.projPos = mul(posW, viewProj).xyww;

    return output;
}