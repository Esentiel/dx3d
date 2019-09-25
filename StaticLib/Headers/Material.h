#pragma once

#include <DirectXMath.h>

namespace Library
{
	class Material
	{
	public:
		Material();
		~Material();

	public:
		DirectX::XMFLOAT4 Emissive; // 16
		DirectX::XMFLOAT4 Ambient; // 16
		DirectX::XMFLOAT4 Diffuse; // 16
		DirectX::XMFLOAT4 Specular; // 16
		//
		float SpecularPower;
		int CalcLight;
		int HasNormalMap;
		int HasSpecularMap; // 4 + 4 + 4 + 4
	};
}


