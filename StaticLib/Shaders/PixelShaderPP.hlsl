Texture2D offScreenTexture : register(t0);
SamplerState simpleSamplerPP : register(s0);

struct PS_INPUT
{
	float2 textCoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 color = offScreenTexture.Sample(simpleSamplerPP, input.textCoord);
    float greyscaleAverage = (color.r + color.g + color.b) / 3.0f;
    color.rgba = float4(greyscaleAverage, greyscaleAverage, greyscaleAverage, color.a);


	return color;
}