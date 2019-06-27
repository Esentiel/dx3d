struct PS_INPUT
{
	float4 projPos : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return float4(0.f, 0.f, 0.f, 0.f);
}