

struct LightSource
{
	float4 lightPos;
	float4 lightDir;
	float4 lightPower;
	int type; // 0 - unused; 1 - directional; 2 - point; 3 - spot;
	float3 padding;
};

struct ResultingLight
{
	float4 diffuse;
	float4 specular;
};

static const float4 ColorWhite = { 1, 1, 1, 1 };
static const float3 ColorBlack = { 0, 0, 0 };
static const float DepthBias = 0.00000005;
static const int MaxLightOnScene = 6;

// Phong model
float4 CalculateDiffuseLight(LightSource light, float3 L, float3 normal)
{
	float NdotL = saturate(dot(L, normal));
    return light.lightPower * NdotL;
}

float4 CalculateSpecularLight(LightSource light, float3 L, float3 eyePos, float3 position, float3 normal, float specularPower)
{
	float3 E = normalize(eyePos - position);
	float3 R = normalize(reflect(-L, normal));
	float RdotE = saturate(dot(R, E));
	float specPow = pow(RdotE, specularPower);
	float4 spec = light.lightPower * specPow;
	
	return spec;
}

ResultingLight ProcessDirectionalLight(LightSource light, float3 eyePos, float3 position, float3 normal, float specularPower)
{
	float3 L = normalize(-light.lightDir.xyz);
	ResultingLight result;
	result.diffuse = CalculateDiffuseLight(light, L, normal);
	result.specular = CalculateSpecularLight(light, L, eyePos, position, normal, specularPower);

	return result;
}

ResultingLight CalculateLight(LightSource lights[MaxLightOnScene], float3 eyePos, float3 position, float3 normal, float specularPower)
{
	ResultingLight summResult = { {0, 0, 0, 0}, {0, 0, 0, 0} };
	for (int i = 0; i < MaxLightOnScene; i++)
	{
		ResultingLight result = { {0, 0, 0, 0}, {0, 0, 0, 0} };
		if (lights[i].type == 1)
		{
			result = ProcessDirectionalLight(lights[i], eyePos, position, normal, specularPower);
		}
		summResult.diffuse += result.diffuse;
		summResult.specular += result.specular;
	}
	summResult.diffuse = saturate(summResult.diffuse);
	summResult.specular = saturate(summResult.specular);

	return summResult;
}
