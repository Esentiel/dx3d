struct LightSource
{
	float3 lightPos;
	float3 lightDir;
	float4 lightPower;
};

static const float4 ColorWhite = { 1, 1, 1, 1 };
static const float3 ColorBlack = { 0, 0, 0 };
static const float DepthBias = 0.00000005;

Texture2D diffuseTexture : register(t0);
Texture2D shadowMap : register(t1);
Texture2D normalMap : register(t3);
Texture2D specularMap : register(t4);
SamplerState simpleSampler : register(s0);

SamplerState DepthMapSampler : register(s1);

cbuffer MVPbuffer : register(b0)
{
    float4x4 mvp;
    float4x4 world;
	float4x4 viewProj;
    float4x4 shadowMapMatrix;
    float diffuseIntensity;
    float emissiveK;
    float ambientK;
    float roughness;
    int calcLight;
    int hasNormalMap;
    int hasSpecularMap;
}

cbuffer PerScene : register(b1)
{
	float3 eyePos;
    LightSource lightS;
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
    float4 textureColor = diffuseTexture.Sample(simpleSampler, input.textCoord);

    if (!calcLight)
    {
        return textureColor;
    }

    float3 bumpNormal;
    if (hasNormalMap)
    {
        float4 bumpMap;
        // normal
        bumpMap = normalMap.Sample(simpleSampler, input.textCoord);
    
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
    
    float4 spec;
    float4 specIntensity;
    float diffuseLight = saturate(dot(normalize(-(lightS.lightDir)), bumpNormal));
    if (diffuseLight > 0)
    {
        

        float3 L = normalize(-(lightS.lightDir));
        float3 E = normalize(eyePos - input.posW);
        float3 H = normalize(L + E);
        
        spec = pow(saturate(dot(bumpNormal, H)), 4);

        
        if (hasSpecularMap)
        {
            specIntensity = specularMap.Sample(simpleSampler, input.textCoord);
            spec *= specIntensity;
        }
        else
        {
            spec *= (1.0 - roughness);
        }
    }

    float4 color = saturate(lightS.lightPower * diffuseLight);
    color = saturate(color + textureColor);

    if (diffuseLight > 0)
    {
        color = saturate(color + spec);
    }

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