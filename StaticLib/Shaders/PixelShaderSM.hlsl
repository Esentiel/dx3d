struct VS_out
{
	float4 vPos : POSITION0;
	float4 pos : SV_Position;
};

float2 ComputeMoments(float Depth) 
{   
    float2 Moments;   // First moment is the depth itself.   
    Moments.x = Depth;   // Compute partial derivatives of depth.    
    float dx = ddx(Depth);   
    float dy = ddy(Depth);   // Compute second moment over the pixel extents.   
    Moments.y = Depth*Depth + 0.25*(dx*dx + dy*dy);   
    
    return Moments; 
    
} 

float2 main(VS_out input) : SV_Target
{
	float2 res = float2(0.f, 0.f);

	if (input.pos.z > 0)
	{
		float dist = length(input.vPos);
		res = ComputeMoments(dist);

		res = float2(input.pos.z, input.pos.z * input.pos.z);
		//res = float2(dist, dist * dist);
	}

	return res;
}