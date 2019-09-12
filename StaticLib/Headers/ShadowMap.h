#pragma once

#include <wrl.h>
#include <string>
#include <DirectXMath.h>

struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11InputLayout;
struct ID3D11Buffer;
struct D3D11_VIEWPORT;

namespace Library
{

	struct LightSource;

	class ShadowMap
	{
	public:
		ShadowMap();
		~ShadowMap();

		void Initialize(int width, int height);
		void Generate(RenderScene * scene);
		void SetLightSource(LightSource * light);
		
		ID3D11Buffer* GetConstMeshLightBuffer() const;
		ID3D11Buffer** GetConstMeshLightBufferRef();
		const DirectX::XMMATRIX* GetViewMatrix();
		virtual const DirectX::XMMATRIX* GetProjection() const;
		ID3D11ShaderResourceView** GetShadowMapRef();
	private:
		void CreateInputLayout();
		void CreateConstLightMeshBuffer();

		std::string m_vertexShaderName;
		LightSource * m_lightSource;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderRes;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_shadowMap;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputlayout;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constMeshLightBuffer;
		
		std::unique_ptr<DirectX::XMMATRIX> m_lightView;
		std::unique_ptr<DirectX::XMMATRIX> m_projection;
		std::unique_ptr<D3D11_VIEWPORT> m_viewport;

		int m_width;
		int m_height;
	};
}
