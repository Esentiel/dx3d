#pragma once

#include <wrl.h>
#include <string>
#include <array>
#include <DirectXMath.h>

struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11InputLayout;
struct ID3D11Buffer;
struct D3D11_VIEWPORT;
struct ID3D11RasterizerState;
struct D3D11_VIEWPORT;

namespace Library
{

	struct LightSource;

	class ShadowMap
	{
	public:
		struct SMCB
		{
			DirectX::XMFLOAT4X4 shadowMapVP[MAX_LIGHT_SOURCES];
		};
	public:
		ShadowMap();
		~ShadowMap();

		void Initialize(int width, int height);
		void Generate(RenderScene * scene);
		void SetLightSource(LightSource * light);
		
		ID3D11Buffer* GetConstMeshLightBuffer(unsigned int id) const;
		ID3D11Buffer** GetConstMeshLightBufferRef(unsigned int id);
		const DirectX::XMMATRIX GetViewMatrix(unsigned int id);
		virtual const DirectX::XMMATRIX GetProjection() const;
		ID3D11ShaderResourceView** GetShadowMapRef();

		ID3D11Buffer* GetConstMeshBuffer() const;
		ID3D11Buffer** GetConstMeshBufferRef();
	private:
		void CreateInputLayout();
		void CreateConstLightMeshBuffer();
		void CreateConstMeshBuffer();
		void CreateRasterState();

		std::string m_vertexShaderName;
		std::array<LightSource*, MAX_LIGHT_SOURCES> m_lightSource;
		std::array<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>, MAX_LIGHT_SOURCES> m_shaderRes;
		std::array<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>, MAX_LIGHT_SOURCES> m_shadowMap;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputlayout;
		std::array<Microsoft::WRL::ComPtr<ID3D11Buffer>, MAX_LIGHT_SOURCES> m_constMeshLightBuffer;
		Microsoft::WRL::ComPtr <ID3D11RasterizerState> m_rasterState;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constMeshBuffer;
		std::array<DirectX::XMFLOAT4X4, MAX_LIGHT_SOURCES> m_lightView;
		DirectX::XMFLOAT4X4 m_projection;
		std::unique_ptr<D3D11_VIEWPORT> m_viewport;

		int m_width;
		int m_height;
	};
}
