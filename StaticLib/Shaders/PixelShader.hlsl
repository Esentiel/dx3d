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
}

struct PS_INPUT
{
	float2 textCoord : TEXCOORD0;
    float3 normal : NORMAL0;
	//float4 projPos : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	//return float4(input.projPos.x / 1000, input.projPos.y / 1000, input.projPos.z / 1000, 1.f);

    float4 color = diffuseTexture.Sample(simpleSampler, input.textCoord) * ambientK;

    return color;
}