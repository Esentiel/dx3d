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
SamplerState simpleSampler : register(s0);

SamplerState DepthMapSampler : register(s1);

cbuffer MVPbuffer : register(b0)
{
    float4x4 mvp;
    float4x4 world;
    float4x4 shadowMapMatrix;
    float diffuseIntensity;
    float emissiveK;
    float ambientK;
    float roughness;
    int calcLight;
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
    float3 posW : POSITION0;
    float4 posSM : POSITION1;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 color = diffuseTexture.Sample(simpleSampler, input.textCoord) * float4(0.8, 0.8, 0.8, 0.8);

    if (!calcLight)
    {
        return color;
    }

    float4 diffuseLight = max(dot(normalize(lightS.lightDir), normalize(input.normalW)), 0) * float4(0.8, 0.8, 0.8, 0.8);

    float3 L = normalize(lightS.lightDir);
    float3 E = normalize(eyePos - input.posW);
    float3 H = normalize(L + E);

    float4 spec = pow(max(dot(input.normalW, H), 0), 0.1) * float4(0.8, 0.8, 0.8, 0.8);
    diffuseLight += spec;

    color *= diffuseLight;
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

    return color;
}