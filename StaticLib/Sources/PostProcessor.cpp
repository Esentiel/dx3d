#include "stdafx.h"
#include "PostProcessor.h"

#include <d3d11.h>

#include "GameException.h"

using namespace Library;

PostProcessor::PostProcessor(int width, int height)
{
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
		throw GameException("IDXGIDevice::CreateTexture2D() failed.", hr);
	}

	// create view for rtv
	if (FAILED(hr = g_D3D->device->CreateRenderTargetView(fullScreenTexture.Get(), nullptr, m_offscreenRtv.GetAddressOf())))
	{
		throw GameException("IDXGIDevice::CreateRenderTargetView() failed.", hr);
	}

	// create view for shader res
	if (FAILED(hr = g_D3D->device->CreateShaderResourceView(fullScreenTexture.Get(), nullptr, m_shaderRes.GetAddressOf())))
	{
		throw GameException("IDXGIDevice::CreateShaderResourceView() failed.", hr);
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
		throw GameException("IDXGIDevice::CreateTexture2D() failed.", hr);
	}

	// create view for dsb
	if (FAILED(hr = g_D3D->device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, m_offscreenDsb.GetAddressOf())))
	{
		throw GameException("IDXGIDevice::CreateDepthStencilView() failed.", hr);
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

void PostProcessor::Start()
{
	g_D3D->deviceCtx->OMSetRenderTargets(1, m_offscreenRtv.GetAddressOf(), m_offscreenDsb.Get());
}
