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
		int HasNormalMap;
		int HasSpecularMap;
		int padding_;// 4 + 4 + 4 + 4
	};
}


