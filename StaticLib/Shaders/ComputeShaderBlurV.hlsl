#include "ShaderDefines.inc"

cbuffer BlurCB : register(b0)
{
	float4 weights[11]; //TODO: 11 is hardcoded
};

Texture2DArray inputTx : register(t0);
RWTexture2DArray<float2> outputTx : register(u0);

#define NUM_THREADS 256

int radius = 5;

[numthreads(1, NUM_THREADS, 1)]
void main( int3 groupThreadID : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	for (int i = 0; i < NumCascades; i++)
	{
		float2 color = float2(0.f, 0.f);

		if (DTid.y > 11 && DTid.y < 700)
		{
			for (int j = 0; j < 11; j++)
			{
				color += weights[j].x * inputTx[float3(DTid.x, DTid.y - radius + j, i)].xy;
			}
		}
		else
		{
			color = inputTx[float3(DTid.x, DTid.y, i)].xy;
		}

        
        outputTx[float3(DTid.x, DTid.y, i)] = color;

    }
}