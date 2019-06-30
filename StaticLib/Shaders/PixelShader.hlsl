Texture2D diffuseTexture : register(t0);
SamplerState simpleSampler : register(s0);

struct PS_INPUT
{
	float2 textCoord : TEXCOORD0;
	//float4 projPos : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	//return float4(input.projPos.x / 1000, input.projPos.y / 1000, input.projPos.z / 1000, 1.f);

	return diffuseTexture.Sample(simpleSampler, input.textCoord);
}