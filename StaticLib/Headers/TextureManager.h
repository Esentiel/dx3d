#pragma once

#include <map>
#include <string>
#include <wrl.h>

struct ID3D11ShaderResourceView;

namespace Library
{
	class TextureManager
	{
	public:
		TextureManager();
		~TextureManager();

		ID3D11ShaderResourceView** GetTexture(const std::string &name);
		void LoadTexture(const std::string &name);
	private:
		std::map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> m_textures;
	};
}
