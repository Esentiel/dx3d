#pragma once

#include <wrl.h>
#include <DirectXMath.h>
#include <memory>

struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;

namespace Library
{
	class SkyBox
	{
	public:
		SkyBox();
		~SkyBox();

		void Draw();
	private:
		void CreateSkySphere();
		void CreateRasterState();
		void CreateDSState();
		void CreateSamplerState();
		void CreateTexture();

		std::unique_ptr<DirectX::XMFLOAT3[]> m_vertices;
		std::unique_ptr<UINT[]> m_indices;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsv;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_cubesTexSamplerState;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubesTextureRes;
	};
}


