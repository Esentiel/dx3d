struct Material
{
	float4 emissive; // 16
	float4 ambient; // 16
    float4 diffuse; // 16
	float4 specular; // 16
	//
    float specularPower;
    int calcLight;
    int hasNormalMap;
    int hasSpecularMap; // 4 + 4 + 4 + 4
};

struct LightSource
{
	float4 lightPos; // 16
	float4 lightDir; // 16
	float4 lightPower; // 16
	//
	int type; // 0 - unused; 1 - directional; 2 - point; 3 - spot;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation; // 4 + 4 + 4 + 4
	//
	float spotAngle;
	float3 padding_; // 4+ 12
};

struct ResultingLight
{
	float4 diffuse;
	float4 specular;
};

static const float4 ColorWhite = { 1, 1, 1, 1 };
static const float3 ColorBlack = { 0, 0, 0 };
static const float DepthBias = 0.00000005;
static const int MaxLightOnScene = 1;

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

float CalculateAttenuation(LightSource light, float d)
{
	return 1.0f / ( light.constantAttenuation + light.linearAttenuation * d + light.quadraticAttenuation * d * d );
}

float CalculateSpotCone(LightSource light, float3 L)
{
	float minCos = cos( light.spotAngle );
    float maxCos = ( minCos + 1.0f ) / 2.0f;
    float cosAngle = dot( light.lightDir.xyz, -L );
    return smoothstep( minCos, maxCos, cosAngle ); 
}

ResultingLight ProcessDirectionalLight(LightSource light, float3 eyePos, float3 position, float3 normal, float specularPower)
{
	float3 L = normalize(-light.lightDir.xyz);
	ResultingLight result;
	result.diffuse = CalculateDiffuseLight(light, L, normal);
	result.specular = CalculateSpecularLight(light, L, eyePos, position, normal, specularPower);

	return result;
}

ResultingLight ProcessPointLight(LightSource light, float3 eyePos, float3 position, float3 normal, float specularPower)
{
	float3 L = (light.lightPos.xyz - position);
    float distance = length(L);
    L = L / distance;
	float attenuation = CalculateAttenuation(light, distance);
	
	ResultingLight result;
	result.diffuse = CalculateDiffuseLight(light, L, normal) * attenuation;
	result.specular = CalculateSpecularLight(light, L, eyePos, position, normal, specularPower) * attenuation;

	return result;
}

ResultingLight ProcessSpotLight(LightSource light, float3 eyePos, float3 position, float3 normal, float specularPower)
{
	float3 L = (light.lightPos.xyz - position);
    float distance = length(L);
    L = L / distance;

	float attenuation = CalculateAttenuation(light, distance);
	float spotIntensity = CalculateSpotCone( light, L );

	ResultingLight result;
	result.diffuse = CalculateDiffuseLight(light, L, normal) * attenuation * spotIntensity;
	result.specular = CalculateSpecularLight(light, L, eyePos, position, normal, specularPower) * attenuation * spotIntensity;

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
		else if (lights[i].type == 2)
		{
			result = ProcessPointLight(lights[i], eyePos, position, normal, specularPower);
		}
		else if (lights[i].type == 3)
		{
			result = ProcessSpotLight(lights[i], eyePos, position, normal, specularPower);
		}
		summResult.diffuse += result.diffuse;
		summResult.specular += result.specular;
	}
	summResult.diffuse = saturate(summResult.diffuse);
	summResult.specular = saturate(summResult.specular);

	return summResult;
}
