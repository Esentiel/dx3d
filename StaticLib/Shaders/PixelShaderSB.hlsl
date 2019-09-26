SamplerState ObjSamplerState : register(s2);
TextureCube SkyMap : register(t1);

struct PS_INPUT
{
    float3 posL : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_Target
{
    return SkyMap.Sample(ObjSamplerState, input.posL);
}