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
	m_lightView(new DirectX::XMMATRIX),
	m_vertexShaderName("VertexShaderSM"),
	m_viewport(new D3D11_VIEWPORT)
{
	CreateInputLayout();
	CreateConstLightMeshBuffer();

	

	
}


ShadowMap::~ShadowMap()
{
}

void ShadowMap::Initialize(int width, int height)
{
	// fix
	ID3D11ShaderResourceView *const pSRV[1] = { NULL };
	g_D3D->deviceCtx->PSSetShaderResources(1, 1, pSRV);

	g_D3D->deviceCtx->RSSetState(m_rasterState.Get());

	if (m_width != width || m_height != height)
	{
		HRESULT hr;

		m_width = width;
		m_height = height;

		// create shaderRes
		D3D11_TEXTURE2D_DESC textureDesc;
		ZeroMemory(&textureDesc, sizeof(textureDesc));
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> fullScreenTexture = nullptr;
		if (FAILED(hr = g_D3D->device->CreateTexture2D(&textureDesc, nullptr, fullScreenTexture.GetAddressOf())))
		{
			throw GameException("IDXGIDevice::CreateTexture2D() failed.", hr);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
		ZeroMemory(&resourceViewDesc, sizeof(resourceViewDesc));
		resourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
		resourceViewDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		resourceViewDesc.Texture2D.MipLevels = 1;

		if (FAILED(hr = g_D3D->device->CreateShaderResourceView(fullScreenTexture.Get(), &resourceViewDesc, m_shaderRes.GetAddressOf())))
		{
			throw GameException("IDXGIDevice::CreateShaderResourceView() failed.", hr);
		}

		// create DSB
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
		ZeroMemory(&depthStencilViewDesc, sizeof
		(depthStencilViewDesc));
		depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		if (FAILED(hr = g_D3D->device->CreateDepthStencilView(fullScreenTexture.Get(), &depthStencilViewDesc, m_shadowMap.GetAddressOf())))
		{
			throw GameException("ShadowMap::CreateDepthStencilView() failed.", hr);
		}

		float aspectRatio = (float)m_width / m_height;
		m_projection.reset(new DirectX::XMMATRIX(DirectX::XMMatrixPerspectiveFovRH(DirectX::XM_PIDIV4, aspectRatio, 0.001f, 1000.0f)));


		m_viewport->Width = (float)m_width;
		m_viewport->Height = (float)m_height;
		m_viewport->TopLeftX = 0.f;
		m_viewport->TopLeftY = 0.f;
		m_viewport->MinDepth = 0.f;
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

	//g_D3D->deviceCtx->RSSetViewports(1, m_viewport.get());

	// PER MESH
	for (auto it = scene->BeginMesh(); it != scene->EndMesh(); ++it)
	{
		auto mesh = (*it).get();
		// update mesh cb
		MeshLightCB meshCb;
		DirectX::XMStoreFloat4x4(&meshCb.WorldViewLightProj, *(mesh->GetModelTransform()) * *(GetViewMatrix()) * *(GetProjection()));
		//DirectX::XMStoreFloat4x4(&meshCb.WorldViewLightProj, *(mesh->GetModelTransform()) * *(g_D3D->camera->GetView()) * *(g_D3D->camera->GetProjection()));

		g_D3D->deviceCtx->UpdateSubresource(GetConstMeshLightBuffer(), 0, nullptr, &meshCb, 0, 0);

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
		g_D3D->deviceCtx->VSSetConstantBuffers(2, 1, GetConstMeshLightBufferRef());

		// PS
		g_D3D->deviceCtx->PSSetShader(NULL, NULL, 0);

		// draw call
		g_D3D->deviceCtx->DrawIndexed(mesh->GetIndexCount(), 0, 0);
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
			throw GameException("CreateInputLayout() failed", hr);
		}
	}
	else
	{
		throw GameException("Mesh::CreateInputLayout() failed as vertexShaderBLOB is null");
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

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&cbDesc, NULL, &m_constMeshLightBuffer)))
	{
		throw GameException("CreateConstLightMeshBuffer(): CreateBuffer() failed", hr);
	}
}

ID3D11Buffer* ShadowMap::GetConstMeshLightBuffer() const
{
	return m_constMeshLightBuffer.Get();
}

ID3D11Buffer** ShadowMap::GetConstMeshLightBufferRef()
{
	return m_constMeshLightBuffer.GetAddressOf();
}

void ShadowMap::SetLightSource(LightSource * light)
{
	m_lightSource = light;

	auto pos = DirectX::XMLoadFloat4(&(light->LightPos));
	auto dir = DirectX::XMVector4Normalize(DirectX::XMVectorNegate(DirectX::XMLoadFloat4(&(light->LightDir))));

	//*m_lightView = DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(.0f, 30.f, 40.0f, 1.f), DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 0.f), DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.f));
	*m_lightView = DirectX::XMMatrixLookToRH(DirectX::XMVectorSet(35.5316048, 67.4720230, 59.0638657, 1.00000000), DirectX::XMVectorSet(-0.350119293, -0.548293114, -0.759467721, 0.000000000), DirectX::XMVectorSet(-0.229548156, 0.836286247, -0.497928649, 0.000000000));
}

const DirectX::XMMATRIX* ShadowMap::GetViewMatrix()
{
	return m_lightView.get();
}

const DirectX::XMMATRIX* Library::ShadowMap::GetProjection() const
{
	return m_projection.get();
}

ID3D11ShaderResourceView** Library::ShadowMap::GetShadowMapRef()
{
	return m_shaderRes.GetAddressOf();
}

void Library::ShadowMap::CreateRasterState()
{
	// rasterizer
	D3D11_RASTERIZER_DESC rasterizerState;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.CullMode = D3D11_CULL_FRONT;
	//rasterizerState.FrontCounterClockwise = false;
	rasterizerState.DepthBias = 100000;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 1.0f;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.ScissorEnable = true;
	rasterizerState.MultisampleEnable = false;
	rasterizerState.AntialiasedLineEnable = false;
	g_D3D->device->CreateRasterizerState(&rasterizerState, &m_rasterState);
}