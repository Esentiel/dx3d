struct LightSource
{
    float3 lightPos;
    float3 lightDir;
    float4 lightPower;
};

SamplerState ObjSamplerState : register(s2);
TextureCube SkyMap : register(t2);

cbuffer MVPbuffer : register(b0)
{
    float4x4 mvp;
    float4x4 world;
    float4x4 shadowMapMatrix;
    float diffuseIntensity;
    float emissiveK;
    float ambientK;
    float roughness;
    int calcLight;
}

cbuffer PerScene : register(b1)
{
    float3 eyePos;
    LightSource lightS;
}

struct PS_INPUT
{
    float3 posL : POSITION0;
};

float4 main(PS_INPUT input) : SV_Target
{
    return SkyMap.Sample(ObjSamplerState, input.posL);
}