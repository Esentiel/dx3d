#include "ShaderDefines.inc"

cbuffer BlurCB : register(b0)
{
	float4 weights[11]; //TODO: 11 is hardcoded
};

Texture2DArray inputTx : register(t0);
RWTexture2DArray<float2> outputTx : register(u0);

#define NUM_THREADS 256

int radius = 5;

[numthreads(NUM_THREADS, 1, NumCascades)]
void main( int3 groupThreadID : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	//for (int i = 0; i < NumCascades; i++)
	{
		float2 color = float2(0.f, 0.f);

		if (DTid.x > 11 && DTid.x < 1269)
		{
			for (int j = 0; j < 11; j++)
			{
				float w = weights[j].x;
				float2 smpl = inputTx[float3(DTid.x - radius + j, DTid.y, DTid.z)].xy;
				color += (w * smpl);
			}
		}
		else
		{
			color = inputTx[float3(DTid.x, DTid.y, DTid.z)].xy;
		}

        outputTx[float3(DTid.x, DTid.y, DTid.z)] = color;
    }
}