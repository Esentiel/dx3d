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

namespace Library
{

	struct LightSource;

	class ShadowMap
	{
	public:
		struct SMCB
		{
			DirectX::XMFLOAT4X4 shadowMapVP[MAX_LIGHT_SOURCES][NUM_CASCADES];
		};
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
		virtual const DirectX::XMMATRIX GetProjection(unsigned int id) const;
		ID3D11ShaderResourceView** GetShadowMapRef();

		ID3D11Buffer* GetConstMeshBuffer() const;
		ID3D11Buffer** GetConstMeshBufferRef();
	private:
		struct Plane
		{
			DirectX::XMFLOAT3 p1, p2, p3, p4; // p1 = right-top, p2 = left-top, p3=bottom-left, p4=bottom-right
		};
		struct Cascade
		{
			DirectX::XMFLOAT3 points[8];
		};
		void CreateInputLayout();
		void CreateConstLightMeshBuffer();
		void CreateConstMeshBuffer();
		void CreateRasterState();
		std::array<Library::ShadowMap::Cascade, NUM_CASCADES> CalcCascades();

		std::string m_vertexShaderName;
		std::array<LightSource, MAX_LIGHT_SOURCES> m_lightSource;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_shaderRes;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_shadowMap;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputlayout;
		std::array<std::array<Microsoft::WRL::ComPtr<ID3D11Buffer>, NUM_CASCADES>, MAX_LIGHT_SOURCES> m_constMeshLightBuffer;
		Microsoft::WRL::ComPtr <ID3D11RasterizerState> m_rasterState;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constMeshBuffer;
		std::array<DirectX::XMFLOAT4X4, MAX_LIGHT_SOURCES> m_lightView;
		DirectX::XMFLOAT4X4 m_projection[NUM_CASCADES];
		std::unique_ptr<D3D11_VIEWPORT> m_viewport;

		int m_width;
		int m_height;
	};
}
