Texture2D offScreenTexture : register(t0);
SamplerState simpleSamplerPP : register(s0);

struct PS_INPUT
{
	float2 textCoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 color = offScreenTexture.Sample(simpleSamplerPP, input.textCoord);

	return color;
}