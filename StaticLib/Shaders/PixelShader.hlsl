#include "ShaderDefines.inc"


Texture2D diffuseTexture : register(t0);
TextureCube SkyMap : register(t1);
Texture2D normalMap : register(t2);
Texture2D specularMap : register(t3);
Texture2DArray shadowMap : register(t4);

SamplerState magLinearWrapSampler : register(s0);
SamplerState DepthMapSampler : register(s1);
SamplerState ObjSamplerState : register(s2);

cbuffer MVPbuffer : register(b0)
{
    float4x4 model; // 64 
    float4x4 view; // 64
    float4x4 projection; // 64
    Material material; // 80
}

cbuffer PerScene : register(b1)
{
	float4 eyePos; // 16
	float4 GlobalAmbient; // 16
    LightSource lightS[MaxLightOnScene]; // 64x6
}

cbuffer ShadowMapVP : register(b2)
{
    float4x4 shadowMapProj[MaxLightOnScene][NumCascades]; // NumCascades
    float4x4 shadowMapView[MaxLightOnScene]; // NumCascades
    float4 limits;
}


struct PS_INPUT
{
	float2 textCoord : TEXCOORD0;
    float3 normalW : NORMAL0;
    float3 tangentsW : TANGENTS0;
    float3 bitangentsW : BITANGENTS0;
    float3 posW : POSITION0;
};


float4 main(PS_INPUT input) : SV_TARGET
{
	// normals
	float4 bumpMap = float4(0.0, 0.0, 0.0, 0.0);
	float3 normal;
    if (material.hasNormalMap)
    {
        
        // normal
        float4 bumpMap = normalMap.Sample(magLinearWrapSampler, input.textCoord);
    
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
        float4 viewPos = mul(float4(input.posW, 1.0), view);
        
        // select correct cascade
        int cascadeIdx;
        for (int j = 0; j < NumCascades; j++)
        {
            if (abs(viewPos.z) < limits[j])
            {
                cascadeIdx = j;
                break;
            }
        }
        
        float4x4 shadowViewProj = mul(shadowMapView[i], shadowMapProj[i][cascadeIdx]);
        
        float4 SMposFinal = mul(float4(input.posW, 1.0), shadowViewProj);
        
        if (SMposFinal.w >= 0.0f)
		{
			if (lightS[i].type == 0)
				continue;    
            
            SMposFinal.xyz /= SMposFinal.w;

            //if (cascadeIdx == 0)
            //{
            //    finalColor.rgb *= float3(1.5, 1.0, 1.0);
            //}
            //else if (cascadeIdx == 1)
            //{
            //    finalColor.rgb *= float3(1.0, 1.5, 1.0);
            //}
            //else if (cascadeIdx == 2)
            //{
            //    finalColor.rgb *= float3(1.0, 1.0, 1.5);
            //}
            
            float pixelDepth = SMposFinal.z;

            float3 textPos = float3(SMposFinal.x, SMposFinal.y, (float) cascadeIdx);
            float2 sampledDepth = shadowMap.Sample(DepthMapSampler, textPos).xy;
                
            bool isShadowed = pixelDepth < 1.0f && pixelDepth > sampledDepth.x;

            if (isShadowed)
            {
                float3 shadow = ColorShadow * ShadowFactor;
                finalColor.rgb *= shadow;
            }
            else
            {
                float variance = (sampledDepth.y) - (sampledDepth.x * sampledDepth.x);
                variance = min(1.0f, max(0.0f, variance + 0.00001f));
                
                float mean = sampledDepth.x;
                float d = pixelDepth - mean;
                
                float p_max = variance / (variance + d * d);
                float fPercentLit = pow(p_max, 4);
                
                float3 shadow = saturate(ColorShadow * (ShadowFactor / fPercentLit));
                //finalColor.rgb = lerp(shadow, finalColor.rgb, fPercentLit);
                finalColor.rgb *= shadow;

            }
        }
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