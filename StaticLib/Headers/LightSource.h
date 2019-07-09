#pragma once

#include <DirectXMath.h>

namespace Library
{
	struct LightSource
	{
		DirectX::XMFLOAT4 LightPos;
		DirectX::XMFLOAT4 LightDir;
		DirectX::XMFLOAT4 LightPower;


		LightSource();
		~LightSource();
	};
}