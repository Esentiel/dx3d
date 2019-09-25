#include "ShaderDefines.inc"

Texture2D specularMap : register(t4);
Texture2D diffuseTexture : register(t0);
Texture2D shadowMap : register(t1);
Texture2D normalMap : register(t3);

SamplerState magLinearWrapSampler : register(s0);
SamplerState DepthMapSampler : register(s1);

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
    float4 posSM : POSITION1;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	// normals
	float3 normal;
    if (material.hasNormalMap)
    {
        float4 bumpMap;
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
	if (input.posSM.w >= 0.0f)
    {
		float width = 0, height = 0;
		shadowMap.GetDimensions(width, height);
		float2 ShadowMapSize = 1 / float2(width, height);

        input.posSM.xyz /= input.posSM.w;
        float pixelDepth = input.posSM.z;
		
        float sampledDepth1 = shadowMap.Sample(DepthMapSampler, float2(input.posSM.x, input.posSM.y)).x + DepthBias;
		float sampledDepth2 = shadowMap.Sample(DepthMapSampler, float2(input.posSM.x, input.posSM.y) + float2(ShadowMapSize.x, 0)).x + DepthBias;
		float sampledDepth3 = shadowMap.Sample(DepthMapSampler, float2(input.posSM.x, input.posSM.y) + float2(0, ShadowMapSize.y)).x + DepthBias;
		float sampledDepth4 = shadowMap.Sample(DepthMapSampler, float2(input.posSM.x, input.posSM.y) + float2(ShadowMapSize.x, ShadowMapSize.y)).x + DepthBias;

		int shadowPCFvalue = 0;
		shadowPCFvalue += (int)(pixelDepth > sampledDepth1);
		shadowPCFvalue += (int)(pixelDepth > sampledDepth2);
		shadowPCFvalue += (int)(pixelDepth > sampledDepth3);
		shadowPCFvalue += (int)(pixelDepth > sampledDepth4);

		float shadowFactorPCF = shadowPCFvalue / 4.0f;

		if (shadowFactorPCF > 0.2)
		{
			float3 shadow = ColorBlack + (1 - shadowFactorPCF);
			finalColor.rgb *= shadow;
		}       
    }

	finalColor.a = 1;
	return finalColor;
}