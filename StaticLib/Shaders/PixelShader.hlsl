#include "ShaderDefines.inc"

Texture2D specularMap : register(t4);
SamplerState magLinearWrapSampler : register(s0);
Texture2D diffuseTexture : register(t0);
Texture2D shadowMap : register(t1);
Texture2D normalMap : register(t3);



SamplerState DepthMapSampler : register(s1);

cbuffer MVPbuffer : register(b0)
{
    float4x4 mvp; // 64 
    float4x4 world; // 64
	float4x4 viewProj; // 64
    float4x4 shadowMapMatrix; // 64
	float4 Emissive; // 16
	float4 Ambient; // 16
    float4 Diffuse; // 16
	float4 Specular; // 16
	//
    float specularPower;
    int calcLight;
    int hasNormalMap;
    int hasSpecularMap; //16
}

cbuffer PerScene : register(b1)
{
	float4 eyePos;
	float4 GlobalAmbient;
    LightSource lightS[MaxLightOnScene];
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
	float3 bumpNormal;
    if (hasNormalMap)
    {
        float4 bumpMap;
        // normal
        bumpMap = normalMap.Sample(magLinearWrapSampler, input.textCoord);
    
        // Expand the range of the normal value from (0, +1) to (-1, +1).
        bumpMap = (bumpMap * 2.0f) - 1.0f;

        // Calculate the normal from the data in the bump map.
        bumpNormal = (bumpMap.x * input.tangentsW) + (bumpMap.y * input.bitangentsW) + (bumpMap.z * input.normalW);

        // Normalize the resulting bump normal.
        bumpNormal = normalize(bumpNormal);
    }
    else
    {
        bumpNormal = normalize(input.normalW);
    }


	ResultingLight result = CalculateLight(lightS, eyePos.xyz, input.posW, bumpNormal, specularPower);
	
	float4 emissive = Emissive;
    float4 ambient = Ambient * GlobalAmbient;
    float4 diffuse = Diffuse * result.diffuse;
	float4 specular = result.specular;
	if (hasSpecularMap)
    {
        float4 specIntensity = specularMap.Sample(magLinearWrapSampler, input.textCoord);
        specular *= specIntensity;
    }
    else
    {
        specular *= Specular;
    }

	float4 textureColor = diffuseTexture.Sample(magLinearWrapSampler, input.textCoord);

    float4 finalColor = ( emissive + ambient + diffuse + specular ) * textureColor;
	// gloss effect / more specular
	finalColor += specular;
	
	finalColor.a = 1;
	return finalColor;
}

float4 main1(PS_INPUT input) : SV_TARGET
{
    float4 textureColor = diffuseTexture.Sample(magLinearWrapSampler, input.textCoord);

    if (!calcLight)
    {
        return textureColor;
    }

    float3 bumpNormal;
    if (hasNormalMap)
    {
        float4 bumpMap;
        // normal
        bumpMap = normalMap.Sample(magLinearWrapSampler, input.textCoord);
    
        // Expand the range of the normal value from (0, +1) to (-1, +1).
        bumpMap = (bumpMap * 2.0f) - 1.0f;

        // Calculate the normal from the data in the bump map.
        bumpNormal = (bumpMap.x * input.tangentsW) + (bumpMap.y * input.bitangentsW) + (bumpMap.z * input.normalW);

        // Normalize the resulting bump normal.
        bumpNormal = normalize(input.normalW);

    }
    else
    {
        bumpNormal = normalize(input.normalW);
    }
    
    float4 spec = float4(0.f, 0.f, 0.f, 0.f);
    float4 specIntensity  = float4(1.f, 1.f, 1.f, 1.f);
    float diffuseLight = saturate(dot(normalize(-(lightS[0].lightDir.xyz)), bumpNormal));
    if (diffuseLight > 0)
    {
        

        float3 L = normalize(-(lightS[0].lightDir.xyz));
        float3 E = normalize(eyePos - input.posW);
        float3 H = normalize(L + E);
        
		float powS = saturate(dot(bumpNormal, H));

        spec = pow(powS, 32);

        
        if (hasSpecularMap)
        {
            specIntensity = specularMap.Sample(magLinearWrapSampler, input.textCoord);
            spec *= specIntensity;
        }
        else
        {
            spec *= (1.0 - 0.5);
        }
    }

    float4 color = saturate(float4(lightS[0].lightPower.xyz, 1.0f) * diffuseLight);
    color = color + spec;

	color = saturate(color* textureColor);

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
			color.rgb *= shadow;
		}       
    }
    color.a = 1;
    return color;
}