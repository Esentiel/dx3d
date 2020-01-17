#include "stdafx.h"
#include "ShadowMap.h"

#include <d3d11.h>
#include <DirectXMath.h>

#include "Mesh.h"
#include "GameException.h"
#include "LightSource.h"
#include "RenderScene.h"
#include "Camera.h"
#include "ShaderManager.h"

using namespace Library;

ShadowMap::ShadowMap() :
	m_vertexShaderName("VertexShaderSM"),
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
			textureDesc.Width = m_width * MAX_LIGHT_SOURCES;
			textureDesc.Height = m_height * NUM_CASCADES;
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 1;
			textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

			Microsoft::WRL::ComPtr<ID3D11Texture2D> fullScreenTexture = nullptr;
			if (FAILED(hr = g_D3D->device->CreateTexture2D(&textureDesc, nullptr, fullScreenTexture.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION("IDXGIDevice::CreateTexture2D() failed.", hr);
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
			ZeroMemory(&resourceViewDesc, sizeof(resourceViewDesc));
			resourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
			resourceViewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
			resourceViewDesc.Texture2D.MipLevels = 1;

			if (FAILED(hr = g_D3D->device->CreateShaderResourceView(fullScreenTexture.Get(), &resourceViewDesc, m_shaderRes.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION("IDXGIDevice::CreateShaderResourceView() failed.", hr);
			}

			// create DSB
			D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
			ZeroMemory(&depthStencilViewDesc, sizeof
			(depthStencilViewDesc));
			depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
			depthStencilViewDesc.Texture2D.MipSlice = 0;

			if (FAILED(hr = g_D3D->device->CreateDepthStencilView(fullScreenTexture.Get(), &depthStencilViewDesc, m_shadowMap.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION("ShadowMap::CreateDepthStencilView() failed.", hr);
			}
		}

		// calc cascades
		auto cascades = CalcCascades();


		//

		float aspectRatio = (float)m_width / m_height;

		DirectX::XMMATRIX P0 = DirectX::XMMatrixPerspectiveFovRH(DirectX::XM_PIDIV4, aspectRatio, 50.0f, 100.0f);
		DirectX::XMMATRIX P1 = DirectX::XMMatrixPerspectiveFovRH(DirectX::XM_PIDIV4, aspectRatio, 100.0f, 150.0f);
		DirectX::XMMATRIX P2 = DirectX::XMMatrixPerspectiveFovRH(DirectX::XM_PIDIV4, aspectRatio, 150.0f, 200.0f);
		DirectX::XMStoreFloat4x4(&m_projection[0], P0);
		DirectX::XMStoreFloat4x4(&m_projection[1], P1);
		DirectX::XMStoreFloat4x4(&m_projection[2], P2);

		m_viewport->Width = (float)m_width;
		m_viewport->Height = (float)m_height;
		m_viewport->TopLeftX = 0.f;
		m_viewport->TopLeftY = 0.f;
		m_viewport->MinDepth = 0.0f;
		m_viewport->MaxDepth = 1.f;
	}
}

void ShadowMap::Generate(RenderScene * scene)
{
	static ID3D11RenderTargetView* nullRenderTargetView = nullptr;
	
	// set off screen rtv and dsb
	g_D3D->deviceCtx->OMSetRenderTargets(1, &nullRenderTargetView, m_shadowMap.Get());

	// clear off screen rt and dsb
	const DirectX::XMVECTORF32 BackgroundColor = { 0.392f,0.584f, 0.929f, 1.0f };
	g_D3D->deviceCtx->ClearDepthStencilView(m_shadowMap.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; i++)
	{
		for (unsigned int j = 0; j < NUM_CASCADES; j++)
		{
			m_viewport->Width = (float)m_width;
			m_viewport->Height = (float)m_height;
			m_viewport->TopLeftX = i * (float)m_width;
			m_viewport->TopLeftY = j * (float)m_height;
			m_viewport->MinDepth = 0.0f;
			m_viewport->MaxDepth = 1.f;

			g_D3D->deviceCtx->RSSetViewports(1, m_viewport.get());

			// scissors test
			D3D11_RECT rects[1];
			rects[0].left = i * (long)m_width;
			rects[0].right = m_width * (i + 1);
			rects[0].top = j * (float)m_height;
			rects[0].bottom = m_height * (j + 1);

			g_D3D->deviceCtx->RSSetScissorRects(1, rects);

			// PER MESH
			for (auto it = scene->BeginMesh(); it != scene->EndMesh(); ++it)
			{
				auto mesh = (*it).get();

				// update mesh cb
				MeshLightCB meshCb;
				DirectX::XMStoreFloat4x4(&meshCb.WorldViewLightProj, mesh->GetModelTransform() * GetViewMatrix(i) * GetProjection(j));

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
				g_D3D->deviceCtx->PSSetShader(NULL, NULL, 0);

				// draw call
				g_D3D->deviceCtx->DrawIndexed(mesh->GetIndexCount(), 0, 0);
			}
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

void ShadowMap::SetLightSource(LightSource * light)
{
	for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; i++)
	{
		if (!light[i].Type)
			continue;

		m_lightSource[i] = &light[i];

		DirectX::XMVECTOR pos;
		DirectX::XMVECTOR dir = DirectX::XMVector4Normalize((DirectX::XMLoadFloat4(&(m_lightSource[i]->LightDir))));
		if (m_lightSource[i]->Type == 1)
		{
			pos = DirectX::XMVectorScale(DirectX::XMVectorNegate(DirectX::XMLoadFloat4(&(m_lightSource[i]->LightDir))), 100.f);
		}
		else
		{
			pos = DirectX::XMLoadFloat4(&(m_lightSource[i]->LightPos));
		}


		DirectX::XMMATRIX V = DirectX::XMMatrixLookToRH(pos, dir, DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.f));
		DirectX::XMStoreFloat4x4(&(m_lightView[i]), V);
	}
}

const DirectX::XMMATRIX ShadowMap::GetViewMatrix(unsigned int id)
{
	return DirectX::XMLoadFloat4x4(&(m_lightView[id]));
}

const DirectX::XMMATRIX Library::ShadowMap::GetProjection(unsigned int id) const
{
	assert(id < NUM_CASCADES);
	return DirectX::XMLoadFloat4x4(&m_projection[id]);
}

ID3D11ShaderResourceView** Library::ShadowMap::GetShadowMapRef()
{
	return m_shaderRes.GetAddressOf();
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
	rasterizerState.DepthClipEnable = true;
	g_D3D->device->CreateRasterizerState(&rasterizerState, &m_rasterState);
}

std::vector<Library::ShadowMap::Cascade> Library::ShadowMap::CalcCascades()
{
	std::vector<Library::ShadowMap::Cascade> result;

	// calc points in View Space
	// say we want to have 3 cascades: 30% - 40% - 30%

	const float vertFov = g_D3D->camera->GetFov();
	const float horFov = 2 * atan(tan(vertFov / 2) * m_width / m_height);
	const float nearZ = g_D3D->camera->GetNear();
	const float farZ = g_D3D->camera->GetFar();

	const float nearX = nearZ * tan(horFov);
	const float farX = farZ * tan(horFov);

	const float nearY = nearZ * tan(vertFov);
	const float farY = farZ * tan(vertFov);

	Cascade cascade1;
	cascade1.nearPlane.p1 = DirectX::XMFLOAT3(nearX, nearY, nearZ);
	cascade1.nearPlane.p2 = DirectX::XMFLOAT3(-nearX, nearY, nearZ);
	cascade1.nearPlane.p3 = DirectX::XMFLOAT3(-nearX, -nearY, nearZ);
	cascade1.nearPlane.p4 = DirectX::XMFLOAT3(nearX, -nearY, nearZ);

	cascade1.farPlane.p1 = DirectX::XMFLOAT3(farX, farY, farZ);
	cascade1.farPlane.p2 = DirectX::XMFLOAT3(-farX, farY, farZ);
	cascade1.farPlane.p3 = DirectX::XMFLOAT3(-farX, -farY, farZ);
	cascade1.farPlane.p4 = DirectX::XMFLOAT3(farX, -farY, farZ);

	// transform into world
	auto viewMx = g_D3D->camera->GetView();
	auto invertedViewMx = DirectX::XMMatrixInverse(NULL, viewMx);

	DirectX::XMVECTOR nearP1Vec = DirectX::XMLoadFloat3(&(cascade1.nearPlane.p1));
	DirectX::XMVECTOR nearP2Vec = DirectX::XMLoadFloat3(&(cascade1.nearPlane.p2));
	DirectX::XMVECTOR nearP3Vec = DirectX::XMLoadFloat3(&(cascade1.nearPlane.p3));
	DirectX::XMVECTOR nearP4Vec = DirectX::XMLoadFloat3(&(cascade1.nearPlane.p4));
	DirectX::XMVECTOR farP1Vec = DirectX::XMLoadFloat3(&(cascade1.farPlane.p1));
	DirectX::XMVECTOR farP2Vec = DirectX::XMLoadFloat3(&(cascade1.farPlane.p2));
	DirectX::XMVECTOR farP3Vec = DirectX::XMLoadFloat3(&(cascade1.farPlane.p3));
	DirectX::XMVECTOR farP4Vec = DirectX::XMLoadFloat3(&(cascade1.farPlane.p4));

	nearP1Vec = DirectX::XMVector3Transform(nearP1Vec, invertedViewMx);
	nearP2Vec = DirectX::XMVector3Transform(nearP2Vec, invertedViewMx);
	nearP3Vec = DirectX::XMVector3Transform(nearP3Vec, invertedViewMx);
	nearP4Vec = DirectX::XMVector3Transform(nearP4Vec, invertedViewMx);
	farP1Vec = DirectX::XMVector3Transform(farP1Vec, invertedViewMx);
	farP2Vec = DirectX::XMVector3Transform(farP2Vec, invertedViewMx);
	farP3Vec = DirectX::XMVector3Transform(farP3Vec, invertedViewMx);
	farP4Vec = DirectX::XMVector3Transform(farP4Vec, invertedViewMx);

	DirectX::XMStoreFloat3(&(cascade1.nearPlane.p1), nearP1Vec);
	DirectX::XMStoreFloat3(&(cascade1.nearPlane.p2), nearP2Vec);
	DirectX::XMStoreFloat3(&(cascade1.nearPlane.p3), nearP3Vec);
	DirectX::XMStoreFloat3(&(cascade1.nearPlane.p4), nearP4Vec);
	DirectX::XMStoreFloat3(&(cascade1.farPlane.p1), farP1Vec);
	DirectX::XMStoreFloat3(&(cascade1.farPlane.p2), farP2Vec);
	DirectX::XMStoreFloat3(&(cascade1.farPlane.p3), farP3Vec);
	DirectX::XMStoreFloat3(&(cascade1.farPlane.p4), farP4Vec);

	result.push_back(cascade1);

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