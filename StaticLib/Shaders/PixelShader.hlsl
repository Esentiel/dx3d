#include "ShaderDefines.inc"


Texture2D diffuseTexture : register(t0);
Texture2D normalMap : register(t2);
Texture2D specularMap : register(t3);
Texture2D shadowMap0 : register(t4);
Texture2D shadowMap1 : register(t5);
Texture2D shadowMap2 : register(t6);
Texture2D shadowMap3 : register(t7);
Texture2D shadowMap4 : register(t8);
Texture2D shadowMap5 : register(t9);

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
    float4 posSM[MaxLightOnScene] : POSITION1;
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

	for (int i = 0; i < MaxLightOnScene; i++)
	{
		if (input.posSM[i].w >= 0.0f)
		{
			float4 posSM = input.posSM[i];
			posSM.xyz /= posSM.w;
			float pixelDepth = posSM.z;


			float width = 0, height = 0;
			float2 ShadowMapSize = 0;
			float sampledDepth1 = 0;
			float sampledDepth2 = 0;
			float sampledDepth3 = 0;
			float sampledDepth4 = 0;

			if (i == 0)
			{
				shadowMap0.GetDimensions(width, height);
				ShadowMapSize = 1 / float2(width, height);

				sampledDepth1 = shadowMap0.Sample(DepthMapSampler, float2(posSM.x, posSM.y)).x + DepthBias;
				sampledDepth2 = shadowMap0.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(ShadowMapSize.x, 0)).x + DepthBias;
				sampledDepth3 = shadowMap0.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(0, ShadowMapSize.y)).x + DepthBias;
				sampledDepth4 = shadowMap0.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(ShadowMapSize.x, ShadowMapSize.y)).x + DepthBias;
			}
			else if (i == 1)
			{
				shadowMap1.GetDimensions(width, height);
				ShadowMapSize = 1 / float2(width, height);

				sampledDepth1 = shadowMap1.Sample(DepthMapSampler, float2(posSM.x, posSM.y)).x + DepthBias;
				sampledDepth2 = shadowMap1.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(ShadowMapSize.x, 0)).x + DepthBias;
				sampledDepth3 = shadowMap1.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(0, ShadowMapSize.y)).x + DepthBias;
				sampledDepth4 = shadowMap1.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(ShadowMapSize.x, ShadowMapSize.y)).x + DepthBias;
			}
			else if (i == 2)
			{
				shadowMap2.GetDimensions(width, height);
				ShadowMapSize = 1 / float2(width, height);

				sampledDepth1 = shadowMap2.Sample(DepthMapSampler, float2(posSM.x, posSM.y)).x + DepthBias;
				sampledDepth2 = shadowMap2.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(ShadowMapSize.x, 0)).x + DepthBias;
				sampledDepth3 = shadowMap2.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(0, ShadowMapSize.y)).x + DepthBias;
				sampledDepth4 = shadowMap2.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(ShadowMapSize.x, ShadowMapSize.y)).x + DepthBias;
			}
			else if (i == 3)
			{
				shadowMap3.GetDimensions(width, height);
				ShadowMapSize = 1 / float2(width, height);

				sampledDepth1 = shadowMap3.Sample(DepthMapSampler, float2(posSM.x, posSM.y)).x + DepthBias;
				sampledDepth2 = shadowMap3.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(ShadowMapSize.x, 0)).x + DepthBias;
				sampledDepth3 = shadowMap3.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(0, ShadowMapSize.y)).x + DepthBias;
				sampledDepth4 = shadowMap3.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(ShadowMapSize.x, ShadowMapSize.y)).x + DepthBias;
			}
			else if (i == 4)
			{
				shadowMap4.GetDimensions(width, height);
				ShadowMapSize = 1 / float2(width, height);

				sampledDepth1 = shadowMap4.Sample(DepthMapSampler, float2(posSM.x, posSM.y)).x + DepthBias;
				sampledDepth2 = shadowMap4.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(ShadowMapSize.x, 0)).x + DepthBias;
				sampledDepth3 = shadowMap4.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(0, ShadowMapSize.y)).x + DepthBias;
				sampledDepth4 = shadowMap4.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(ShadowMapSize.x, ShadowMapSize.y)).x + DepthBias;
			}
			else if (i == 5)
			{
				shadowMap5.GetDimensions(width, height);
				ShadowMapSize = 1 / float2(width, height);

				sampledDepth1 = shadowMap5.Sample(DepthMapSampler, float2(posSM.x, posSM.y)).x + DepthBias;
				sampledDepth2 = shadowMap5.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(ShadowMapSize.x, 0)).x + DepthBias;
				sampledDepth3 = shadowMap5.Sample(DepthMapSampler, float2(posSM.x, posSM.y) + float2(0, ShadowMapSize.y)).x + DepthBias;
				sampledDepth4 = shadowMap5.Sample(DepthMapSampler, float2(posSM.x, input.posSM[i].y) + float2(ShadowMapSize.x, ShadowMapSize.y)).x + DepthBias;
			}

			
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
    }

	finalColor.a = 1;
	return finalColor;
}