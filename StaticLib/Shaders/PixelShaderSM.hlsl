

float2 main(float4 pos : SV_Position) : SV_Target
{
	float2 res = float2(0.f, 0.f);
	res.x = pos.z;
	pos.y = pos.z * pos.z;
	return res;
}