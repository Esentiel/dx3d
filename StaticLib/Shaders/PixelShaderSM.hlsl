

float2 main(float4 pos : SV_Position) : SV_Target
{
	float2 res = float2(0.f, 0.f);

	if (pos.z > 0)
		res = float2(pos.z, pos.z * pos.z);

	return res;
}