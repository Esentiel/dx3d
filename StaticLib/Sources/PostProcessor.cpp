#include "stdafx.h"
#include "PostProcessor.h"

#include <d3d11.h>
#include <DirectXMath.h>

#include "ShaderManager.h"
#include "GameException.h"
#include "Camera.h"

using namespace Library;

static Microsoft::WRL::ComPtr<ID3D11SamplerState> s_sampler;
extern void CreateSampler(ID3D11SamplerState ** smapler);


struct VertexPP
{
	VertexPP(DirectX::XMFLOAT3 pos_, DirectX::XMFLOAT2 textCoord_) : pos(std::move(pos_)), textCoord(std::move(textCoord_)) {};
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 textCoord;
};

static const VertexPP s_verticesPP[4] = 
{
	VertexPP(DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f)),
	VertexPP(DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)),
	VertexPP(DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f)),
	VertexPP(DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f))
};

static const UINT s_indices[] = 
{
	0, 1, 2, 
	0, 2, 3
};





PostProcessor::PostProcessor(int width, int height)
{
	m_vertexShaderName = "VertexShaderPP";
	m_pixelShaderName = "PixelShaderPP";

	// create texture
	D3D11_TEXTURE2D_DESC fullScreenTextureDesc;
	ZeroMemory(&fullScreenTextureDesc, sizeof(fullScreenTextureDesc));
	fullScreenTextureDesc.Width = width;
	fullScreenTextureDesc.Height = height;
	fullScreenTextureDesc.MipLevels = 1;
	fullScreenTextureDesc.ArraySize = 1;
	fullScreenTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	fullScreenTextureDesc.SampleDesc.Count = 1;
	fullScreenTextureDesc.SampleDesc.Quality = 0;
	fullScreenTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	fullScreenTextureDesc.Usage = D3D11_USAGE_DEFAULT;

	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> fullScreenTexture = nullptr;
	if (FAILED(hr = g_D3D->device->CreateTexture2D(&fullScreenTextureDesc, nullptr, fullScreenTexture.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("IDXGIDevice::CreateTexture2D() failed.", hr);
	}

	// create view for rtv
	if (FAILED(hr = g_D3D->device->CreateRenderTargetView(fullScreenTexture.Get(), nullptr, m_offscreenRtv.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("IDXGIDevice::CreateRenderTargetView() failed.", hr);
	}

	// create view for shader res
	if (FAILED(hr = g_D3D->device->CreateShaderResourceView(fullScreenTexture.Get(), nullptr, m_shaderRes.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("IDXGIDevice::CreateShaderResourceView() failed.", hr);
	}

	//create texture for dsb
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer = nullptr;
	if (FAILED(hr = g_D3D->device->CreateTexture2D(&depthStencilDesc, nullptr, depthStencilBuffer.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("IDXGIDevice::CreateTexture2D() failed.", hr);
	}

	// create view for dsb
	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2DMS);
	if (FAILED(hr = g_D3D->device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilViewDesc, m_offscreenDsb.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("IDXGIDevice::CreateDepthStencilView() failed.", hr);
	}
}


PostProcessor::~PostProcessor()
{
}

ID3D11ShaderResourceView* PostProcessor::GetShaderRes()
{
	return m_shaderRes.Get();
}

ID3D11RenderTargetView* PostProcessor::GetOffscreenRtv()
{
	return m_offscreenRtv.Get();
}

ID3D11DepthStencilView* PostProcessor::GetOffscreenDsb()
{
	return m_offscreenDsb.Get();
}

void Library::PostProcessor::Draw()
{
	g_D3D->deviceCtx->RSSetState(m_rasterState.Get());

	// IA
	UINT stride = sizeof(VertexPP);
	UINT offset = 0;
	g_D3D->deviceCtx->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	g_D3D->deviceCtx->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	g_D3D->deviceCtx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_D3D->deviceCtx->IASetInputLayout(m_inputlayout.Get());

	// VS
	ID3D11VertexShader* vs = g_D3D->shaderMgr->GetVertexShader(m_vertexShaderName);
	g_D3D->deviceCtx->VSSetShader(vs, NULL, 0); //todo: mocking: ASSUMPTION: we use same vs for each

	// PS
	ID3D11PixelShader* ps = g_D3D->shaderMgr->GetPixelShader(m_pixelShaderName);
	g_D3D->deviceCtx->PSSetShader(ps, NULL, 0);
	g_D3D->deviceCtx->PSSetSamplers(0, 1, s_sampler.GetAddressOf());
	g_D3D->deviceCtx->PSSetShaderResources(0, 1, m_shaderRes.GetAddressOf());

	// draw call
	g_D3D->deviceCtx->DrawIndexed(ARRAYSIZE(s_indices), 0, 0);
}

void Library::PostProcessor::Begin()
{
	// reset
	ID3D11ShaderResourceView *const pSRV[1] = { NULL };
	g_D3D->deviceCtx->PSSetShaderResources(0, 1, pSRV);

	// set off screen rtv and dsb
	g_D3D->deviceCtx->OMSetRenderTargets(1, m_offscreenRtv.GetAddressOf(), m_offscreenDsb.Get());

	// clear off screen rt and dsb
	const DirectX::XMVECTORF32 BackgroundColor = { 0.392f,0.584f, 0.929f, 1.0f };
	g_D3D->deviceCtx->ClearRenderTargetView(m_offscreenRtv.Get(), reinterpret_cast<const float*>(&BackgroundColor));
	g_D3D->deviceCtx->ClearDepthStencilView(m_offscreenDsb.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//g_D3D->camera->UpdateViewport();
}

void Library::PostProcessor::Initialize()
{
	CreateInputLayout();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateSampler(s_sampler.GetAddressOf());
}

void Library::PostProcessor::CreateInputLayout()
{
	// setup IA layout
	HRESULT hr;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

void Library::PostProcessor::CreateVertexBuffer()
{
	m_vertexBuffer.Reset();

	CD3D11_BUFFER_DESC vDesc(
		sizeof(VertexPP) * ARRAYSIZE(s_verticesPP),
		D3D11_BIND_VERTEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA vData;
	ZeroMemory(&vData, sizeof(D3D11_SUBRESOURCE_DATA));
	vData.pSysMem = s_verticesPP;
	vData.SysMemPitch = 0;
	vData.SysMemSlicePitch = 0;

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&vDesc, &vData, m_vertexBuffer.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("PostProcessor::CreateVertexBuffer(): CreateBuffer() failed", hr);
	}
}

void Library::PostProcessor::CreateIndexBuffer()
{
	m_indexBuffer.Reset();

	CD3D11_BUFFER_DESC iDesc(
		sizeof(UINT) * ARRAYSIZE(s_indices),
		D3D11_BIND_INDEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA iData;
	ZeroMemory(&iData, sizeof(D3D11_SUBRESOURCE_DATA));
	iData.pSysMem = s_indices;
	iData.SysMemPitch = 0;
	iData.SysMemSlicePitch = 0;

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&iDesc, &iData, m_indexBuffer.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("PostProcessor::CreateIndexBuffer(): CreateBuffer() failed", hr);
	}
}

void Library::PostProcessor::CreateRasterState()
{
	// rasterizer
	D3D11_RASTERIZER_DESC rasterizerState;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.CullMode = D3D11_CULL_BACK;
	rasterizerState.FrontCounterClockwise = false;
	rasterizerState.DepthBias = false;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.ScissorEnable = true;
	rasterizerState.MultisampleEnable = true;
	rasterizerState.AntialiasedLineEnable = false;
	g_D3D->device->CreateRasterizerState(&rasterizerState, &m_rasterState);
}
