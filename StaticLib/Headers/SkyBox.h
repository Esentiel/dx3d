#pragma once

#include <wrl.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>
#include <string>

struct ID3D11RasterizerState;
struct ID3D11DepthStencilState;
struct ID3D11Buffer;
struct ID3D11InputLayout;
struct ID3D11ShaderResourceView;
struct ID3D11SamplerState;

namespace Library
{
	class Transformations;

	class SkyBox
	{
	public:
		SkyBox();
		~SkyBox();

		void Draw(RenderScene * renderScene);
	private:
		void CreateSkySphere();
		void CreateRasterState();
		void CreateDSState();
		void CreateSamplerState();
		void CreateTexture();
		void CreateConstMeshBuffer();
		void CreateInputLayout();
		void CreateVertexBuffer();
		void CreateIndexBuffer();

		std::vector<DirectX::XMFLOAT3> m_vertices;
		std::vector<UINT> m_indices;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_dsv;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputlayout;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_cubesTexSamplerState;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_cubesTextureRes;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constMeshBuffer;

		const std::string m_textureName = "grasscube1024.dds";
		const std::string m_VSName = "VertexShaderSB";
		const std::string m_PSName = "PixelShaderSB";

		std::unique_ptr<Transformations> m_transformations;
	};
}


