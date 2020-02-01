#include "stdafx.h"
#include "ShadowMap.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <cmath>

#include "Mesh.h"
#include "GameException.h"
#include "LightSource.h"
#include "RenderScene.h"
#include "Camera.h"
#include "ShaderManager.h"
#include "Transformations.h"

using namespace Library;

ShadowMap::ShadowMap() :
	m_vertexShaderName("VertexShaderSM"),
	m_pixelShaderName("PixelShaderSM"),
	m_viewport(new D3D11_VIEWPORT)
{
	CreateInputLayout();
	CreateConstLightMeshBuffer();
	CreateConstMeshBuffer();
	CreateRasterState();
}


ShadowMap::~ShadowMap()
{
}

void ShadowMap::Initialize(int width, int height)
{
	// fix
	ID3D11ShaderResourceView *const pSRV[1] = { NULL };
	
	g_D3D->deviceCtx->PSSetShaderResources(4, 1, pSRV);

	g_D3D->deviceCtx->RSSetState(m_rasterState.Get());

	if (m_width != width || m_height != height)
	{
		//for (size_t i = 0; i < MAX_LIGHT_SOURCES; i++)
		{
			HRESULT hr;

			m_width = width;
			m_height = height;

			// create shaderRes
			D3D11_TEXTURE2D_DESC textureDesc;
			ZeroMemory(&textureDesc, sizeof(textureDesc));
			textureDesc.Width = m_width;
			textureDesc.Height = m_height;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			textureDesc.SampleDesc.Count = g_D3D->sampleDesc.Count;
			textureDesc.SampleDesc.Quality = g_D3D->sampleDesc.Quality;
			textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

			Microsoft::WRL::ComPtr<ID3D11Texture2D> fullScreenTexture = nullptr;
			if (FAILED(hr = g_D3D->device->CreateTexture2D(&textureDesc, nullptr, fullScreenTexture.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION("IDXGIDevice::CreateTexture2D() failed.", hr);
			}

			// create DSB
			D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
			ZeroMemory(&depthStencilViewDesc, sizeof
			(depthStencilViewDesc));
			depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			depthStencilViewDesc.Texture2D.MipSlice = 0;

			if (FAILED(hr = g_D3D->device->CreateDepthStencilView(fullScreenTexture.Get(), &depthStencilViewDesc, m_shadowMapDSB.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION("ShadowMap::CreateDepthStencilView() failed.", hr);
			}

			// rtv for Variance SM
			D3D11_TEXTURE2D_DESC fullScreenTextureDesc;
			ZeroMemory(&fullScreenTextureDesc, sizeof(fullScreenTextureDesc));
			fullScreenTextureDesc.Width = m_width;
			fullScreenTextureDesc.Height = m_height;
			fullScreenTextureDesc.MipLevels = 1;
			fullScreenTextureDesc.ArraySize = 1;
			fullScreenTextureDesc.Format = DXGI_FORMAT_R8G8_UNORM;
			fullScreenTextureDesc.SampleDesc.Count = g_D3D->sampleDesc.Count;
			fullScreenTextureDesc.SampleDesc.Quality = g_D3D->sampleDesc.Quality;
			fullScreenTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
			fullScreenTextureDesc.Usage = D3D11_USAGE_DEFAULT;

			if (FAILED(hr = g_D3D->device->CreateTexture2D(&fullScreenTextureDesc, nullptr, m_fullScreenTextureRTV.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION("IDXGIDevice::CreateTexture2D() failed.", hr);
			}

			CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2DMS);
			if (FAILED(hr = g_D3D->device->CreateRenderTargetView(m_fullScreenTextureRTV.Get(), &renderTargetViewDesc, m_rtv.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION("CreateRenderTargetView() failed", hr);
			}

			// create SRV

			D3D11_TEXTURE2D_DESC fullScreenTextureDescSRV;
			ZeroMemory(&fullScreenTextureDescSRV, sizeof(fullScreenTextureDescSRV));
			fullScreenTextureDescSRV.Width = m_width;
			fullScreenTextureDescSRV.Height = m_height;
			fullScreenTextureDescSRV.MipLevels = 1;
			fullScreenTextureDescSRV.ArraySize = NUM_CASCADES;
			fullScreenTextureDescSRV.Format = DXGI_FORMAT_R8G8_UNORM;
			fullScreenTextureDescSRV.SampleDesc.Count = 1;
			fullScreenTextureDescSRV.SampleDesc.Quality = 0;
			fullScreenTextureDescSRV.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			fullScreenTextureDescSRV.Usage = D3D11_USAGE_DEFAULT;

			if (FAILED(hr = g_D3D->device->CreateTexture2D(&fullScreenTextureDescSRV, nullptr, m_fullScreenTextureSRV.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION("IDXGIDevice::CreateTexture2D() failed.", hr);
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
			viewDesc.Format = DXGI_FORMAT_R8G8_UNORM;
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.MostDetailedMip = 0;
			viewDesc.Texture2DArray.MipLevels = 1;
			viewDesc.Texture2DArray.FirstArraySlice = 0;
			viewDesc.Texture2DArray.ArraySize = NUM_CASCADES;

			for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
			{
				if (FAILED(hr = g_D3D->device->CreateShaderResourceView(m_fullScreenTextureSRV.Get(), &viewDesc, m_shaderResRTV[i].GetAddressOf())))
				{
					THROW_GAME_EXCEPTION("IDXGIDevice::CreateShaderResourceView() failed.", hr);
				}
			}
		}

	}
}

void ShadowMap::Generate(RenderScene * scene)
{
	CalcProjections();
	
	// set off screen rtv and dsb
	g_D3D->deviceCtx->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_shadowMapDSB.Get());

	m_viewport->Width = (float)m_width;
	m_viewport->Height = (float)m_height;
	m_viewport->TopLeftX = 0;
	m_viewport->TopLeftY = 0;
	m_viewport->MinDepth = 0.0f;
	m_viewport->MaxDepth = 1.f;

	g_D3D->deviceCtx->RSSetViewports(1, m_viewport.get());

	// scissors test
	D3D11_RECT rects[1];
	rects[0].left = 0;
	rects[0].right = m_width;
	rects[0].top = 0;
	rects[0].bottom = m_height;

	g_D3D->deviceCtx->RSSetScissorRects(1, rects);

	for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; i++)
	{
		for (unsigned int j = 0; j < NUM_CASCADES; j++)
		{
			// clear off screen rt and dsb
			const float baseColor[]{ 1.0f, 1.0f };
			g_D3D->deviceCtx->ClearRenderTargetView(m_rtv.Get(), baseColor);
			g_D3D->deviceCtx->ClearDepthStencilView(m_shadowMapDSB.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
			
			// PER MESH
			for (auto it = scene->BeginMesh(); it != scene->EndMesh(); ++it)
			{
				auto mesh = (*it).get();

				// update mesh cb
				MeshLightCB meshCb;
				DirectX::XMStoreFloat4x4(&meshCb.WorldViewLightProj, mesh->GetModelTransform() * GetViewMatrix(i) * GetProjection(i, j));

				g_D3D->deviceCtx->UpdateSubresource(GetConstMeshLightBuffer(i, j), 0, nullptr, &meshCb, 0, 0);

				// IA
				UINT stride = sizeof(DirectX::XMFLOAT3);
				UINT offset = 0;
				g_D3D->deviceCtx->IASetVertexBuffers(0, 1, mesh->GetVertexLightBufferRef(), &stride, &offset);
				g_D3D->deviceCtx->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
				g_D3D->deviceCtx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				g_D3D->deviceCtx->IASetInputLayout(m_inputlayout.Get());

				// VS
				ID3D11VertexShader* vs = g_D3D->shaderMgr->GetVertexShader(m_vertexShaderName);
				g_D3D->deviceCtx->VSSetShader(vs, NULL, 0); //todo: mocking: ASSUMPTION: we use same vs for each
				g_D3D->deviceCtx->VSSetConstantBuffers(2, 1, GetConstMeshLightBufferRef(i, j));

				// PS
				ID3D11PixelShader* ps = g_D3D->shaderMgr->GetPixelShader(m_pixelShaderName);
				g_D3D->deviceCtx->PSSetShader(ps, NULL, 0);

				// draw call
				g_D3D->deviceCtx->DrawIndexed(mesh->GetIndexCount(), 0, 0);
			}

			// resolve MSAA RT
			UINT subResSrc = D3D11CalcSubresource(0, 0, g_D3D->sampleDesc.Count);
			UINT subResSrv = D3D11CalcSubresource(0, j, 1);
			g_D3D->deviceCtx->ResolveSubresource(m_fullScreenTextureSRV.Get(), subResSrv, m_fullScreenTextureRTV.Get(), subResSrc, DXGI_FORMAT_R8G8_UNORM);
		}
	}
}

void ShadowMap::CreateInputLayout()
{
	// setup IA layout
	HRESULT hr;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3DBlob* vertexShaderBLOB = g_D3D->shaderMgr->GetShaderBLOB(m_vertexShaderName);
	if (vertexShaderBLOB)
	{
		if (FAILED(hr = g_D3D->device->CreateInputLayout(layout, _countof(layout), vertexShaderBLOB->GetBufferPointer(), vertexShaderBLOB->GetBufferSize(), m_inputlayout.GetAddressOf())))
		{
			THROW_GAME_EXCEPTION("CreateInputLayout() failed", hr);
		}
	}
	else
	{
		THROW_GAME_EXCEPTION_SIMPLE("Mesh::CreateInputLayout() failed as vertexShaderBLOB is null");
	}
}


void ShadowMap::CreateConstLightMeshBuffer()
{
	MeshLightCB VsConstData;

	auto size = (UINT)std::ceil(sizeof(VsConstData) / 16.f) * 16;

	CD3D11_BUFFER_DESC cbDesc(
		size,
		D3D11_BIND_CONSTANT_BUFFER
	);

	for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; i++)
	{
		for (unsigned int j = 0; j < NUM_CASCADES; j++)
		{
			HRESULT hr;
			if (FAILED(hr = g_D3D->device->CreateBuffer(&cbDesc, NULL, m_constMeshLightBuffer[i][j].GetAddressOf())))
			{
				THROW_GAME_EXCEPTION("CreateConstLightMeshBuffer(): CreateBuffer() failed", hr);
			}
		}
	}
}

ID3D11Buffer* ShadowMap::GetConstMeshLightBuffer(unsigned int id, unsigned int id2) const
{
	return m_constMeshLightBuffer[id][id2].Get();
}

ID3D11Buffer** ShadowMap::GetConstMeshLightBufferRef(unsigned int id, unsigned int id2)
{
	return m_constMeshLightBuffer[id][id2].GetAddressOf();
}

void ShadowMap::SetLightSource(const LightSource * light)
{
	for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; i++)
	{
		if (!light[i].Type)
			continue;

		m_lightSource[i] = light[i];

		DirectX::XMVECTOR pos;
		DirectX::XMVECTOR dir = DirectX::XMVector4Normalize((DirectX::XMLoadFloat4(&(m_lightSource[i].LightDir))));

		if (light[i].Type == 1)
		{
			DirectX::XMFLOAT3 posF(0.f, 0.f, 0.f);
			pos = DirectX::XMLoadFloat3(&posF);
		}
		else
		{
			pos = DirectX::XMLoadFloat4(&(m_lightSource[i].LightPos));
		}



		DirectX::XMMATRIX V = DirectX::XMMatrixLookToLH(pos, dir, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.f));
		DirectX::XMStoreFloat4x4(&(m_lightView[i]), V);
	}
}

const DirectX::XMMATRIX ShadowMap::GetViewMatrix(unsigned int id)
{
	return DirectX::XMLoadFloat4x4(&(m_lightView[id]));
}

const float* Library::ShadowMap::GetLimits(int lightIdx) const
{
	return m_cascadeLimits[lightIdx];
}

void Library::ShadowMap::CalcProjections()
{
	// calc cascades
	auto cascades = CalcCascades();

	for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
	{
		auto cascadesPerLight = cascades[i];
		for (int j = 0; j < NUM_CASCADES; j++)
		{
			const Cascade cascade = cascadesPerLight[j];
			float minX = cascade.points[0].x, minY = cascade.points[0].y, maxX = cascade.points[0].x, maxY = cascade.points[0].y, minZ = cascade.points[0].z, maxZ = cascade.points[0].z;

			for (int k = 0; k < 8; k++)
			{
				minX = min(minX, cascade.points[k].x);
				minY = min(minY, cascade.points[k].y);
				minZ = min(minZ, cascade.points[k].z);
				maxX = max(maxX, cascade.points[k].x);
				maxY = max(maxY, cascade.points[k].y);
				maxZ = max(maxZ, cascade.points[k].z);
			}

			DirectX::XMMATRIX P = DirectX::XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);
			DirectX::XMStoreFloat4x4(&m_projection[i][j], P);
		}
	}
}

const DirectX::XMMATRIX Library::ShadowMap::GetProjection(unsigned int lightIdx, unsigned int cascadeIdx) const
{
	assert(lightIdx < MAX_LIGHT_SOURCES);
	assert(cascadeIdx < NUM_CASCADES);
	return DirectX::XMLoadFloat4x4(&m_projection[lightIdx][cascadeIdx]);
}

ID3D11ShaderResourceView** Library::ShadowMap::GetShadowMapRef()
{
	return m_shaderResRTV[0].GetAddressOf();
}

void Library::ShadowMap::CreateRasterState()
{
	// rasterizer
	D3D11_RASTERIZER_DESC rasterizerState;
	//ZeroMemory(&rasterizerState, sizeof(rasterizerState));
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.CullMode = D3D11_CULL_BACK;
	rasterizerState.DepthBias = 10000;
	rasterizerState.DepthBiasClamp = 0.f;
	rasterizerState.SlopeScaledDepthBias = 1.f;
	rasterizerState.ScissorEnable = true;
	rasterizerState.DepthClipEnable = false;
	rasterizerState.MultisampleEnable = true;
	g_D3D->device->CreateRasterizerState(&rasterizerState, &m_rasterState);
}

std::array<std::array<Library::ShadowMap::Cascade, NUM_CASCADES>, MAX_LIGHT_SOURCES> Library::ShadowMap::CalcCascades()
{
	std::array<std::array<Library::ShadowMap::Cascade, NUM_CASCADES>, MAX_LIGHT_SOURCES> result;

	const float nearestZ = g_D3D->camera->GetNear();
	const float farestZ = g_D3D->camera->GetFar();

	const float vertFovTan = tanf(g_D3D->camera->GetFov() / 2.0f);
	const float ar = (float)m_width / m_height;
	const float horFovTan = vertFovTan * ar;

	auto viewMx = g_D3D->camera->GetView();
	auto invertedViewMx = DirectX::XMMatrixInverse(NULL, viewMx);

	for (int i = 0; i < MAX_LIGHT_SOURCES; i++)
	{
		DirectX::XMMATRIX lightViewMx = GetViewMatrix(i); // TODO: assume we got a single light source

		float nearZ = nearestZ;
		float farZ = (farestZ - nearestZ) * 0.15f + nearZ; // 15% - 25% - 60%
		// transform into world
		for (int j = 0; j < NUM_CASCADES; j++)
		{
			// calc x and y
			const float nearX = nearZ * horFovTan;
			const float nearY = nearZ * vertFovTan;
			const float farX = farZ * horFovTan;
			const float farY = farZ * vertFovTan;

			// cascades
			Cascade cascade;
			cascade.points[0] = DirectX::XMFLOAT3(nearX, nearY, nearZ);
			cascade.points[1] = DirectX::XMFLOAT3(-nearX, nearY, nearZ);
			cascade.points[2] = DirectX::XMFLOAT3(-nearX, -nearY, nearZ);
			cascade.points[3] = DirectX::XMFLOAT3(nearX, -nearY, nearZ);

			cascade.points[4] = DirectX::XMFLOAT3(farX, farY, farZ);
			cascade.points[5] = DirectX::XMFLOAT3(-farX, farY, farZ);
			cascade.points[6] = DirectX::XMFLOAT3(-farX, -farY, farZ);
			cascade.points[7] = DirectX::XMFLOAT3(farX, -farY, farZ);

			DirectX::XMVECTOR vectorPoints[8];

			for (int k = 0; k < 8; k++)
			{
				vectorPoints[k] = DirectX::XMLoadFloat3(&cascade.points[k]);
				// transform into world
				vectorPoints[k] = DirectX::XMVector3Transform(vectorPoints[k], invertedViewMx);
				// transfrom into light view space
				vectorPoints[k] = DirectX::XMVector3Transform(vectorPoints[k], lightViewMx);

				DirectX::XMStoreFloat3(&(cascade.points[k]), vectorPoints[k]);
			}

			result[i][j] = cascade;
			m_cascadeLimits[i][j] = farZ;

			if (j == 0)
			{
				nearZ = farZ;
				farZ = (farestZ - nearestZ) * 0.4f + nearZ; // 15% - 15% - 60%
			}
			else if (j == 1)
			{
				nearZ = farZ;
				farZ = farestZ; // 15% - 35% - 50%
			}
		}
	}

	return result;
}

void ShadowMap::CreateConstMeshBuffer()
{
	SMCB VsConstData;
	auto size = (UINT)std::ceil(sizeof(VsConstData) / 16.f) * 16;

	CD3D11_BUFFER_DESC cbDesc(
		size,
		D3D11_BIND_CONSTANT_BUFFER
	);

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&cbDesc, NULL, &m_constMeshBuffer)))
	{
		THROW_GAME_EXCEPTION("CreateConstMeshBuffer(): CreateBuffer() failed", hr);
	}
}

ID3D11Buffer* ShadowMap::GetConstMeshBuffer() const
{
	return m_constMeshBuffer.Get();
}

ID3D11Buffer** ShadowMap::GetConstMeshBufferRef()
{
	return m_constMeshBuffer.GetAddressOf();
}