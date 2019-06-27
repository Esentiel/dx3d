cbuffer MVPbuffer : register(b0)
{
	matrix mvp;
}

struct VS_IN
{
	float3 position : POSITION;
};

struct VS_OUT
{
	float4 projPos : SV_POSITION;
};

VS_OUT main(VS_IN input) : SV_POSITION
{
	VS_OUT output;
	output.projPos = float4(input.position, 1.0f);
	output.projPos = mul(output.projPos, mvp);
	
	return output;
}