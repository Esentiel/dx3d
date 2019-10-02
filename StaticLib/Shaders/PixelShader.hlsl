#include "ShaderDefines.inc"


Texture2D diffuseTexture : register(t0);
TextureCube SkyMap : register(t1);
Texture2D normalMap : register(t2);
Texture2D specularMap : register(t3);
Texture2D shadowMap[6] : register(t4);

SamplerState magLinearWrapSampler : register(s0);
SamplerState DepthMapSampler : register(s1);
SamplerState ObjSamplerState : register(s2);

cbuffer MVPbuffer : register(b0)
{
    float4x4 mvp; // 16x4 
    float4x4 world; // 16x4
	float4x4 viewProj; // 16x4
    float4x4 shadowMapMatrix; // 16x4
	Material material; // 80
}

cbuffer PerScene : register(b1)
{
	float4 eyePos; // 16
	float4 GlobalAmbient; // 16
    LightSource lightS[MaxLightOnScene]; // 64x6
}

struct PS_INPUT
{
	float2 textCoord : TEXCOORD0;
    float3 normalW : NORMAL0;
    float3 tangentsW : TANGENTS0;
    float3 bitangentsW : BITANGENTS0;
    float3 posW : POSITION0;
    float4 posSM[MaxLightOnScene] : POSITION1;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	// normals
	float4 bumpMap;
	float3 normal;
    if (material.hasNormalMap)
    {
        
        // normal
        bumpMap = normalMap.Sample(magLinearWrapSampler, input.textCoord);
    
        // Expand the range of the normal value from (0, +1) to (-1, +1).
        bumpMap = (bumpMap * 2.0f) - 1.0f;

        // Calculate the normal from the data in the bump map.
        normal = (bumpMap.x * input.tangentsW) + (bumpMap.y * input.bitangentsW) + (bumpMap.z * input.normalW);

        // Normalize the resulting bump normal.
        normal = normalize(normal);
    }
    else
    {
        normal = normalize(input.normalW);
    }


	ResultingLight result = CalculateLight(lightS, eyePos.xyz, input.posW, normal, material.specularPower);
	
	float4 emissive = material.emissive;
    float4 ambient = material.ambient * GlobalAmbient;
    float4 diffuse = material.diffuse * result.diffuse;
	float4 specular = result.specular;
	if (material.hasSpecularMap)
    {
        float4 specIntensity = specularMap.Sample(magLinearWrapSampler, input.textCoord);
        specular *= specIntensity;
    }
    else
    {
        specular *= material.specular;
    }

	float4 textureColor = diffuseTexture.Sample(magLinearWrapSampler, input.textCoord);

    float4 finalColor = ( emissive + ambient + diffuse + specular ) * textureColor;

	// shadows

	float width = 0, height = 0;
	float2 ShadowMapSize = 0;
	float sampledDepth1 = 0;
	float sampledDepth2 = 0;
	float sampledDepth3 = 0;
	float sampledDepth4 = 0;
	float shadowFactorPCF = 0;

	for (int i = 0; i < MaxLightOnScene; i++)
	{
		if (input.posSM[i].w >= 0.0f)
		{
			float4 posSM = input.posSM[i];
			posSM.xyz /= posSM.w;
			float pixelDepth = posSM.z;

            shadowMap[i].GetDimensions(width, height);
			ShadowMapSize = 1 / float2(width, height);

			sampledDepth1 = shadowMap[i].Sample(DepthMapSampler, float2(posSM.x, posSM.y)).x;
			sampledDepth2 = shadowMap[i].Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(ShadowMapSize.x, 0)).x;
			sampledDepth3 = shadowMap[i].Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(0, ShadowMapSize.y)).x;
			sampledDepth4 = shadowMap[i].Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(ShadowMapSize.x, ShadowMapSize.y)).x;

			int shadowPCFvalue = 0;
			shadowPCFvalue += (int)(pixelDepth < 1.0f && pixelDepth > sampledDepth1);
            shadowPCFvalue += (int) (pixelDepth < 1.0f && pixelDepth > sampledDepth2);
            shadowPCFvalue += (int) (pixelDepth < 1.0f && pixelDepth > sampledDepth3);
            shadowPCFvalue += (int) (pixelDepth < 1.0f && pixelDepth > sampledDepth4);

			shadowFactorPCF += shadowPCFvalue / 4.0f;
		}  
    }

	if (shadowFactorPCF > 0.2)
	{
		float3 shadow = shadowFactorPCF/MaxLightOnScene;
		finalColor.rgb -= shadow;
	} 

	float shininess = (1.0f - material.roughness);
	if (material.hasNormalMap)
    {
		shininess *= bumpMap.a;
	}
	float3 toEyeW = normalize(eyePos.xyz - input.posW);
	float3 r = reflect(-toEyeW, normal);
	float4 reflectionColor = SkyMap.Sample(ObjSamplerState, r);
	float3 fresnelR0 = { 0.01f, 0.01f, 0.01f };
	float3 fresnelFactor = SchlickFresnel(fresnelR0, normal, r);

	finalColor.rgb += (shininess * fresnelFactor * reflectionColor.rgb);

	finalColor.a = 1;
	return finalColor;
}