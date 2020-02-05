#include "stdafx.h"
#include "Blur.h"

#include <d3d11.h>
#include "GameException.h"
#include <ShaderManager.h>


Library::Blur::Blur() :
	m_computeShaderName("ComputeShaderBlur"),
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
	}
}

void Library::Blur::Execute(int x, int y, int z)
{
	//Blur
	ID3D11ComputeShader* cs = nullptr;
	cs = g_D3D->shaderMgr->GetComputeShader(m_computeShaderName);
	assert(cs);

	g_D3D->deviceCtx->CSSetShader(cs, NULL, NULL);
	// set buffer with Gaussian resulting array
	g_D3D->deviceCtx->CSSetShaderResources(0, 1, m_shaderResView.GetAddressOf());
	g_D3D->deviceCtx->CSSetUnorderedAccessViews(0, 1, m_viewUA.GetAddressOf(), 0);
	g_D3D->deviceCtx->Dispatch(x, y, z);
}

void Library::Blur::CopyResult(ID3D11Texture2D *resultingTx)
{
	g_D3D->deviceCtx->CopyResource(resultingTx, m_textureUA.Get());
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
