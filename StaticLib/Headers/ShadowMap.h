#pragma once

#include <wrl.h>
#include <string>
#include <array>
#include <DirectXMath.h>
#include <vector>

#include "LightSource.h"

struct ID3D11ShaderResourceView;
struct ID3D11DepthStencilView;
struct ID3D11InputLayout;
struct ID3D11Buffer;
struct D3D11_VIEWPORT;
struct ID3D11RasterizerState;
struct D3D11_VIEWPORT;
struct ID3D11RenderTargetView;
struct ID3D11Texture2D;
struct ID3D11UnorderedAccessView;

namespace Library
{

	struct LightSource;
	class Blur;

	class ShadowMap
	{
	public:
		struct SMCB
		{
			DirectX::XMFLOAT4X4 shadowMapProj[MAX_LIGHT_SOURCES][NUM_CASCADES];
			DirectX::XMFLOAT4X4 shadowMapView[MAX_LIGHT_SOURCES];
			float limits[2][4];
		};
		const float* GetLimits(int lightIdx) const;
	public:
		ShadowMap();
		~ShadowMap();

		void Initialize(int width, int height);
		void Generate(RenderScene * scene);
		void SetLightSource(const LightSource * light);
		void CalcProjections();
		
		ID3D11Buffer* GetConstMeshLightBuffer(unsigned int id, unsigned int id2) const;
		ID3D11Buffer** GetConstMeshLightBufferRef(unsigned int id, unsigned int id2);
		const DirectX::XMMATRIX GetViewMatrix(unsigned int id);
		virtual const DirectX::XMMATRIX GetProjection(unsigned int lightIdx, unsigned int cascadeIdx) const;
		ID3D11ShaderResourceView** GetShadowMapRef();

		ID3D11Buffer* GetConstMeshBuffer() const;
		ID3D11Buffer** GetConstMeshBufferRef();
	private:
		struct Cascade
		{
			DirectX::XMFLOAT3 points[8]; // p1 = right-top, p2 = left-top, p3=bottom-left, p4=bottom-right
		};
		void CreateInputLayout();
		void CreateConstLightMeshBuffer();
		void CreateConstMeshBuffer();
		void CreateRasterState();
		std::array<std::array<Library::ShadowMap::Cascade, NUM_CASCADES>, MAX_LIGHT_SOURCES> CalcCascades();

		std::string m_vertexShaderName;
		std::string m_pixelShaderName;
		
		std::array<LightSource, MAX_LIGHT_SOURCES> m_lightSource;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_fullScreenTextureRTV;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_fullScreenTextureSRV;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderResRTV[MAX_LIGHT_SOURCES];
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_shadowMapDSB;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputlayout;
		std::array<std::array<Microsoft::WRL::ComPtr<ID3D11Buffer>, NUM_CASCADES>, MAX_LIGHT_SOURCES> m_constMeshLightBuffer;
		Microsoft::WRL::ComPtr <ID3D11RasterizerState> m_rasterState;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constMeshBuffer;
		std::array<DirectX::XMFLOAT4X4, MAX_LIGHT_SOURCES> m_lightView;
		DirectX::XMFLOAT4X4 m_projection[MAX_LIGHT_SOURCES][NUM_CASCADES];
		std::unique_ptr<D3D11_VIEWPORT> m_viewport;
		std::unique_ptr<Blur> m_blur;

		float m_cascadeLimits[MAX_LIGHT_SOURCES][NUM_CASCADES];
		int m_width;
		int m_height;
	};
}
