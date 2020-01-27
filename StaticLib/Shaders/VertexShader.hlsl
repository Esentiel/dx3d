#include "ShaderDefines.inc"

cbuffer MVPbuffer : register(b0)
{
    float4x4 model; // 64 
    float4x4 view; // 64
	float4x4 projection; // 64
	Material material; // 80
}


struct VS_IN
{
	float3 position : POSITION;
	float2 textCoord : TEXTCOORD;
	float3 normal : NORMAL;
    float3 tangents : TANGENTS;
    float3 bitangents : BITANGENTS;
};

struct VS_OUT
{
	float2 textCoord : TEXCOORD0;
    float3 normalW : NORMAL0;
    float3 tangentsW : TANGENTS0;
    float3 bitangentsW : BITANGENTS0;
    float3 posW : POSITION0;
	float4 projPos : SV_POSITION;
};

VS_OUT main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;
    
    float4x4 viewProj = mul(view, projection);
	output.textCoord = float2(input.textCoord.x, 1.0 - input.textCoord.y);
    output.normalW = mul(float4(input.normal, 0.0), model).xyz;
    output.tangentsW = mul(float4(input.tangents, 0.0), model).xyz;
    output.bitangentsW = mul(float4(input.bitangents, 0.0), model).xyz;
    output.posW = mul(float4(input.position, 1.0), model).xyz;
    output.projPos = mul(float4(output.posW, 1.0), viewProj);

	return output;
}