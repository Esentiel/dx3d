#include "stdafx.h"
#include "Blur.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <algorithm>
#include "GameException.h"
#include "ShaderManager.h"


Library::Blur::Blur() :
	m_computeShaderNameH("ComputeShaderBlurH"),
	m_computeShaderNameV("ComputeShaderBlurV"),
	m_params(-1, -1, -1, -1)
{
}


Library::Blur::~Blur()
{
}

void Library::Blur::Initialize(int width, int height, int format, int arraySize, ID3D11Texture2D * textureSR, ID3D11ShaderResourceView * viewSR)
{
	if (m_params.HasUpdates(width, height, format, arraySize))
	{
		HRESULT hr;

		// UAV
		D3D11_TEXTURE2D_DESC textureDescUA;
		ZeroMemory(&textureDescUA, sizeof(textureDescUA));
		textureDescUA.Width = width;
		textureDescUA.Height = height;
		textureDescUA.MipLevels = 1;
		textureDescUA.ArraySize = arraySize;
		textureDescUA.Format = (DXGI_FORMAT)format;//DXGI_FORMAT_R8G8_UNORM;
		textureDescUA.SampleDesc.Count = 1;
		textureDescUA.SampleDesc.Quality = 0;
		textureDescUA.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
		textureDescUA.Usage = D3D11_USAGE_DEFAULT;

		if (FAILED(hr = g_D3D->device->CreateTexture2D(&textureDescUA, nullptr, m_textureUA.GetAddressOf())))
		{
			THROW_GAME_EXCEPTION("IDXGIDevice::CreateTexture2D() failed.", hr);
		}

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
		ZeroMemory(&uavDesc, sizeof(uavDesc));
		uavDesc.Format = (DXGI_FORMAT)format;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.MipSlice = 0;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.ArraySize = arraySize;

		if (FAILED(hr = g_D3D->device->CreateUnorderedAccessView(m_textureUA.Get(), &uavDesc, m_viewUA.GetAddressOf())))
		{
			THROW_GAME_EXCEPTION("IDXGIDevice::CreateUnorderedAccessView() failed.", hr);
		}

		if (!textureSR)
		{
			// SVR
			textureDescUA.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			if (FAILED(hr = g_D3D->device->CreateTexture2D(&textureDescUA, nullptr, m_textureSRV.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION("IDXGIDevice::CreateTexture2D() failed.", hr);
			}
		}
		else
		{
			m_textureSRV.Reset();
			m_textureSRV = textureSR;
		}

		if (!viewSR)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
			ZeroMemory(&viewDesc, sizeof(viewDesc));
			viewDesc.Format = (DXGI_FORMAT)format;
			viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			viewDesc.Texture2DArray.MostDetailedMip = 0;
			viewDesc.Texture2DArray.MipLevels = 1;
			viewDesc.Texture2DArray.FirstArraySlice = 0;
			viewDesc.Texture2DArray.ArraySize = arraySize;

			if (FAILED(hr = g_D3D->device->CreateShaderResourceView(m_textureSRV.Get(), &viewDesc, m_shaderResView.GetAddressOf()))) // TODO: need texturearray per Light
			{
				THROW_GAME_EXCEPTION("IDXGIDevice::CreateShaderResourceView() failed.", hr);
			}
		}
		else
		{
			m_shaderResView.Reset();
			m_shaderResView = viewSR;
		}

		// Intermediate drv
		textureDescUA.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		if (FAILED(hr = g_D3D->device->CreateTexture2D(&textureDescUA, nullptr, m_textureIntermediateSRV.GetAddressOf())))
		{
			THROW_GAME_EXCEPTION("IDXGIDevice::CreateTexture2D() failed.", hr);
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDesc;
		ZeroMemory(&viewDesc, sizeof(viewDesc));
		viewDesc.Format = (DXGI_FORMAT)format;
		viewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		viewDesc.Texture2DArray.MostDetailedMip = 0;
		viewDesc.Texture2DArray.MipLevels = 1;
		viewDesc.Texture2DArray.FirstArraySlice = 0;
		viewDesc.Texture2DArray.ArraySize = arraySize;

		if (FAILED(hr = g_D3D->device->CreateShaderResourceView(m_textureIntermediateSRV.Get(), &viewDesc, m_shaderResIntermediateView.GetAddressOf()))) // TODO: need texturearray per Light
		{
			THROW_GAME_EXCEPTION("IDXGIDevice::CreateShaderResourceView() failed.", hr);
		}

		// buffer for weights
		int sizeCb = (int)std::ceil(sizeof(BlurCB) / 16.f) * 16;

		CD3D11_BUFFER_DESC cbDesc(
			sizeCb,
			D3D11_BIND_CONSTANT_BUFFER
		);

		if (FAILED(hr = g_D3D->device->CreateBuffer(&cbDesc, 0, m_blurCBuffer.GetAddressOf())))
		{
			THROW_GAME_EXCEPTION("IDXGIDevice::CreateBuffer() failed.", hr);
		}
	}
}

void Library::Blur::Execute()
{
	auto weights = GetWeights(11); // TODO: hardcoded 11
	BlurCB cb;
	for (int i = 0; i < 11; i++)
	{
		cb.weights[i].x = weights[i];
	}


	g_D3D->deviceCtx->UpdateSubresource(m_blurCBuffer.Get(), 0, NULL, &cb, 0, 0);
	ID3D11ComputeShader* cs = nullptr;
	
	// Horiz Blur
	cs = g_D3D->shaderMgr->GetComputeShader(m_computeShaderNameH);
	assert(cs);

	g_D3D->deviceCtx->CSSetShader(cs, NULL, NULL);
	

	g_D3D->deviceCtx->CSSetConstantBuffers(0, 1, m_blurCBuffer.GetAddressOf());
	g_D3D->deviceCtx->CSSetShaderResources(0, 1, m_shaderResView.GetAddressOf());
	g_D3D->deviceCtx->CSSetUnorderedAccessViews(0, 1, m_viewUA.GetAddressOf(), 0);
	g_D3D->deviceCtx->Dispatch(m_params.width/256, m_params.height, 1);

	g_D3D->deviceCtx->CopyResource(m_textureIntermediateSRV.Get(), m_textureUA.Get());
	
	// Vertical Blur
	cs = nullptr;
	cs = g_D3D->shaderMgr->GetComputeShader(m_computeShaderNameV);
	assert(cs);

	g_D3D->deviceCtx->CSSetConstantBuffers(0, 1, m_blurCBuffer.GetAddressOf());
	g_D3D->deviceCtx->CSSetShaderResources(1, 1, m_shaderResIntermediateView.GetAddressOf());
	g_D3D->deviceCtx->CSSetUnorderedAccessViews(0, 1, m_viewUA.GetAddressOf(), 0);
	g_D3D->deviceCtx->Dispatch(m_params.width, m_params.height/256, 1);
}

void Library::Blur::CopyResult(ID3D11Texture2D *resultingTx)
{
	g_D3D->deviceCtx->CopyResource(resultingTx, m_textureUA.Get());
}

std::vector<float> Library::Blur::GetWeights(int size) const
{
	// Gaussian distribution
	auto fn = [](int size)
	{
		std::vector<float> results(size);

		int iterations = (size - 1) / 2;
		for (int i = 0; i < iterations + 1; i++)
		{
			float val = (1.f / sqrtf((2.f * DirectX::XM_PI))) * powf(2.71828f, (-0.5f * powf((float)i, 2.f)));

			results[iterations + i] = val;

			if (i > 0)
				results[iterations - i] = val;
		}

		float sum = 0;
		for (const auto w : results)
		{
			sum += w;
		}

		for (auto &w : results)
		{
			w /= sum;
		}

		sum = 0;
		for (const auto w : results)
		{
			sum += w;
		}

		return results;
	};

	return fn(size);
}

bool Library::Blur::InitParams::HasUpdates(int width_, int height_, int format_, int arraySize_)
{
	bool res = false;

	if (width != width_)
	{
		width = width_;
		res = true;
	}
	if (height != height_)
	{
		height = height_;
		res = true;
	}
	if (format != format_)
	{
		format = format_;
		res = true;
	}
	if (arraySize == arraySize_)
	{
		arraySize = arraySize_;
		res = true;
	}

	return res;
}
