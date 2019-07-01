#pragma once

#include <DirectXMath.h>

namespace Library
{
	struct LightSource
	{
		DirectX::XMFLOAT3 LightPos;
		DirectX::XMFLOAT3 LightDir;
		DirectX::XMFLOAT4 LightPower;


		LightSource();
		~LightSource();
	};
}