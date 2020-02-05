#pragma once

#include <wrl.h>
#include <string>


struct ID3D11Texture2D;
struct ID3D11UnorderedAccessView;
struct ID3D11ShaderResourceView;

namespace Library
{
	class Blur
	{
	public:
		Blur();
		~Blur();

		void Initialize(int width, int height, int format, int arraySize, ID3D11Texture2D * textureSR = nullptr, ID3D11ShaderResourceView * viewSR = nullptr);

		void Execute(int x = 1, int y = 1, int z = 1);
		void CopyResult(ID3D11Texture2D *resultingTx);
	private:
		struct InitParams
		{
			int width;
			int height;
			int format;
			int arraySize;
			
			InitParams(int width_, int height_, int format_, int arraySize_) : width(width_), height(height_), format(format_), arraySize(arraySize_) {}
			bool HasUpdates(int width_, int height_, int format_, int arraySize_);
		};
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_textureUA;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_textureSRV;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_viewUA;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResView;
		std::string m_computeShaderName;

		InitParams m_params;
	};
}


