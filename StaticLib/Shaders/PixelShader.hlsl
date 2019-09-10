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
    float4x4 world;
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
	float2 textCoord : TEXCOORD0;
    float3 normalW : NORMAL0;
    float3 posW : POSITION0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = diffuseTexture.Sample(simpleSampler, input.textCoord) * float4(0.8, 0.8, 0.8, 0.8);

    if (!calcLight)
    {
        return color;
    }

    float4 diffuseLight = max(dot(normalize(lightS.lightDir), normalize(input.normalW)), 0) * float4(0.8, 0.8, 0.8, 0.8);

    float3 L = normalize(lightS.lightDir);
    float3 E = normalize(eyePos - input.posW);
    float3 H = normalize(L + E);

    float4 spec = pow(max(dot(input.normalW, H), 0), 0.1) * float4(0.8, 0.8, 0.8, 0.8);
    diffuseLight += spec;

    color *= diffuseLight;

    return color;
}