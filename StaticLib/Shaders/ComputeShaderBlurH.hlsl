#include "ShaderDefines.inc"

cbuffer BlurCB : register(b0)
{
	float4 weights[11]; //TODO: 11 is hardcoded
};

Texture2DArray inputTx : register(t0);
RWTexture2DArray<float2> outputTx : register(u0);

#define NUM_THREADS 256

int radius = 5;

[numthreads(NUM_THREADS, 1, 1)]
void main( int3 groupThreadID : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	for (int i = 0; i < NumCascades; i++)
	{
		float color = float(0.f);

		if (DTid.x > 11 && DTid.x < 1269)
		{
			for (int j = 0; j < 11; j++)
			{
				float w = weights[j].x;
				float smpl = inputTx[float3(DTid.x - radius + j, DTid.y, i)].y;
				color += (w * smpl);
			}
		}
		else
		{
			color = inputTx[float3(DTid.x, DTid.y, i)].y;
		}

        outputTx[float3(DTid.x, DTid.y, i)] = float2(inputTx[float3(DTid.x, DTid.y, i)].x, color);
    }
}