struct LightSource
{
	float3 lightPos;
	float3 lightDir;
	float4 lightPower;
};

Texture2D diffuseTexture : register(t0);
SamplerState simpleSampler : register(s0);

cbuffer MVPbuffer : register(b0)
{
    float4x4 mvp;
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

struct PS_INPUT
{
	float2 textCoord : TEXCOORD0;
    float3 normal : NORMAL0;
	//float4 projPos : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = diffuseTexture.Sample(simpleSampler, input.textCoord) * ambientK;

    float4 diffuseLight = max(dot(normalize(lightS.lightDir), input.normal), 0) * diffuseIntensity;

    color.xyz *= diffuseLight;

    return color;
}