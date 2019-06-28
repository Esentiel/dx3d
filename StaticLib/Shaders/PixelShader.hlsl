struct PS_INPUT
{
	float4 projPos : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	return float4(input.projPos.x / 1000, input.projPos.y / 1000, input.projPos.z / 1000, 1.f);
}