#include "ShaderDefines.inc"

cbuffer MVLPbuffer : register(b2)
{
    float4x4 mvlp;
	float4x4 mvl;
}

struct VS_out
{
	float4 vPos : POSITION0;
	float4 pos : SV_Position;
};

VS_out main(float3 position : POSITION)
{
	VS_out output = (VS_out)0;
	output.pos = mul(float4(position, 1.0), mvlp);
    output.vPos = mul(float4(position, 1.0), mvl);

	return output;
}