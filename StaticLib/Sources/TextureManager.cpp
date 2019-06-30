#include "stdafx.h"
#include "TextureManager.h"

#include <d3d11.h>
#include <DDSTextureLoader.h>

#include "GameException.h"

static const std::string s_texturesDir = "../Content/Textures/";

using namespace Library;

TextureManager::TextureManager()
{
}


TextureManager::~TextureManager()
{
}

void TextureManager::LoadTexture(const std::string &name)
{
	assert(g_D3D->device);
	assert(g_D3D->deviceCtx);

	if (m_textures.find(name) == m_textures.end())
	{
		Microsoft::WRL::ComPtr<ID3D11Resource> texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;

		std::string path = s_texturesDir + name;
		std::wstring pathW(path.begin(), path.end());

		HRESULT hr;
		if (FAILED(hr = DirectX::CreateDDSTextureFromFile(g_D3D->device, pathW.c_str(), texture.GetAddressOf(), textureView.GetAddressOf())))
		{
			throw GameException("LoadTexture(): CreateDDSTextureFromFile() failed", hr);
		}

		m_textures.insert({ name , textureView });
	}
}

ID3D11ShaderResourceView** Library::TextureManager::GetTexture(const std::string &name)
{
	auto it = m_textures.find(name);
	if (it != m_textures.end())
		return it->second.GetAddressOf();
	
	std::string text("GetTexture() failed for texture: " + name);
	throw GameException(text.c_str());
}
