#pragma once

#include <wrl.h>
#include <string>
#include <vector>

#include <DirectXMath.h>

struct ID3D11Texture2D;
struct ID3D11UnorderedAccessView;
struct ID3D11ShaderResourceView;
struct ID3D11Buffer;

namespace Library
{
	class Blur
	{
	public:
		Blur();
		~Blur();

		void Initialize(int width, int height, int format, int arraySize, ID3D11Texture2D * textureSR = nullptr, ID3D11ShaderResourceView * viewSR = nullptr);

		void Execute();
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

		struct BlurCB
		{
			DirectX::XMFLOAT4 weights[11]; //TODO: 11 is hardcoded
		};

		std::vector<float> GetWeights(int size) const; 

		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_textureUA;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_textureSRV;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_textureIntermediateSRV;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_viewUA;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResIntermediateView;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_blurCBuffer;
		std::string m_computeShaderNameH;
		std::string m_computeShaderNameV;

		InitParams m_params;
	};
}


