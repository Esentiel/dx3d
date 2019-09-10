cbuffer MVLPbuffer : register(b2)
{
    float4x4 mvlp;
}

float4 main(float3 position : POSITION) : SV_Position
{
    return mul(float4(position, 1.0), mvlp);
}