cbuffer MVPbuffer : register(b0)
{
	float4x4 mvp;
	float diffuseIntensity;
	float emissiveK;
	float ambientK;
	float roughness;
}

struct VS_IN
{
	float3 position : POSITION;
	float2 textCoord : TEXTCOORD;
	float3 normal : NORMAL;
};

struct VS_OUT
{
	float2 textCoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 posW : POSITION0;
	float4 projPos : SV_POSITION;
};

VS_OUT main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;
	float4 pos= float4(input.position, 1.0);
	output.projPos = mul(pos, mvp);
	output.textCoord = float2(input.textCoord.x, 1.0 - input.textCoord.y);
    output.normal = normalize(input.normal);
    output.posW = input.position;

	return output;
}