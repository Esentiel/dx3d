cbuffer MVPbuffer : register(b0)
{
	float4x4 mvp;
}

struct VS_IN
{
	float3 position : POSITION;
	float2 textCoord : TEXTCOORD;
};

struct VS_OUT
{
	float2 textCoord : TEXCOORD0;
	float4 projPos : SV_POSITION;
};

VS_OUT main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;
	float4 pos= float4(input.position, 1.0);
	output.projPos = mul(pos, mvp);
	output.textCoord = input.textCoord;

	return output;
}