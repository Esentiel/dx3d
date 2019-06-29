#pragma once

#include <string>
#include <map>
#include <wrl.h>

struct ID3D10Blob;
typedef ID3D10Blob ID3DBlob;
struct ID3D11VertexShader;
struct ID3D11PixelShader;
struct ID3D11Device;

namespace Library
{
	class ShaderManager
	{
	public:
		ShaderManager();
		~ShaderManager();

		ID3DBlob* GetShaderBLOB(const std::string &name) const;
		ID3D11VertexShader* GetVertexShader(const std::string &name) const;
		ID3D11PixelShader* GetPixelShader(const std::string &name) const;

		void Initialize();
	private:
		std::map<std::string, Microsoft::WRL::ComPtr<ID3DBlob>> m_shaders;
		std::map<std::string, Microsoft::WRL::ComPtr<ID3D11VertexShader>> m_vertexShaders;
		std::map<std::string, Microsoft::WRL::ComPtr<ID3D11PixelShader>> m_pixelShaders;
	};
}