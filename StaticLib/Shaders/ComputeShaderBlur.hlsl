Texture2DArray inputTx : register(t0);
RWTexture2DArray<float2> outputTx : register(u0);

[numthreads(8, 8, 3)]
void main( int3 groupThreadID : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	outputTx[DTid.xyz] = inputTx[DTid.xyz].xy;
}