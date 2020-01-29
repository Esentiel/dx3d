

float2 main(float4 pos : SV_Position) : SV_Target
{
    float2 res = float2(pos.z, pos.z * pos.z);
	//res.x = pos.z;
 //   pos.y = pos.z;
	return res;
}