#include "ShaderDefines.inc"

Texture2DArray inputTx : register(t0);
RWTexture2DArray<float2> outputTx : register(u0);

#define NUM_THREADS 256

[numthreads(NUM_THREADS, 1, 1)]
void main( int3 groupThreadID : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	for (int i = 0; i < NumCascades; i++)
	{
		float3 coords = float3(DTid.x, DTid.y, i);
		outputTx[coords] = inputTx[coords].xy;
	}
}