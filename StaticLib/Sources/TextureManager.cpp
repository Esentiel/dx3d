#include "stdafx.h"
#include "TextureManager.h"

#include <d3d11.h>
#include <DDSTextureLoader.h>
#include <filesystem>

#include "GameException.h"

using namespace Library;

TextureManager::TextureManager()
{
	std::filesystem::path p = std::filesystem::path(g_D3D->executablePath);
	m_textureFolderPath = p.parent_path().parent_path().parent_path().parent_path().string() + "\\Content\\Textures\\";
}


TextureManager::~TextureManager()
{
}

void TextureManager::LoadTexture(const std::string &name, ID3D11Texture2D** texture)
{
	assert(g_D3D->device);
	assert(g_D3D->deviceCtx);

	if (name.size() && m_textures.find(name) == m_textures.end())
	{
		Microsoft::WRL::ComPtr<ID3D11Texture2D> internalTexture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureView;
		HRESULT hr;

		std::string path = m_textureFolderPath + name;
		std::wstring pathW(path.begin(), path.end());

		if (texture)
		{
			if (FAILED(hr = DirectX::CreateDDSTextureFromFile(g_D3D->device, pathW.c_str(), (ID3D11Resource**)texture, textureView.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION("LoadTexture(): CreateDDSTextureFromFile() failed", hr);
			}
		}
		else
		{
			if (FAILED(hr = DirectX::CreateDDSTextureFromFile(g_D3D->device, pathW.c_str(), (ID3D11Resource**)internalTexture.GetAddressOf(), textureView.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION("LoadTexture(): CreateDDSTextureFromFile() failed", hr);
			}
		}
		

		m_textures.insert({ name , textureView });
	}
}

ID3D11ShaderResourceView** Library::TextureManager::GetTexture(const std::string &name)
{
	auto it = m_textures.find(name);
	if (it != m_textures.end())
		return it->second.GetAddressOf();

	return nullptr;
}
