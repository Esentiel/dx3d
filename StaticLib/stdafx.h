// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <assert.h>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace Library
{
	typedef struct SAMPLE_DESC
	{
		unsigned int Count;
		unsigned int Quality;
	} SAMPLE_DESC;

	constexpr unsigned int MAX_LIGHT_SOURCES = 1;
	constexpr unsigned int NUM_CASCADES = 3;

	class ShaderManager;
	class RenderScene;
	class TextureManager;
	class FileManager;
	class Camera;
	struct GD3DApp
	{
		ID3D11Device* device;
		ID3D11DeviceContext* deviceCtx;
		ShaderManager* shaderMgr;
		TextureManager* textureMgr;
		FileManager* fileMgr;
		RenderScene* renderScene;
		Camera* camera;
		wchar_t * executablePath;
		SAMPLE_DESC sampleDesc;
	};

	extern GD3DApp * g_D3D;
}

// reference additional headers your program requires here
