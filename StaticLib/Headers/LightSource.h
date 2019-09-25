#pragma once

#include <DirectXMath.h>

namespace Library
{
	struct LightSource
	{
		DirectX::XMFLOAT4 LightPos;
		DirectX::XMFLOAT4 LightDir;
		DirectX::XMFLOAT4 LightPower;
		int Type;  // 0 - unused; 1 - directional; 2 - point; 3 - spot;
		int alignment1, alignment2, alignment3;


		LightSource();
		~LightSource();
	};
}