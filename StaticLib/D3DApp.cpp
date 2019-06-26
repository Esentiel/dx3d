#include "stdafx.h"
#include "D3DApp.h"

#include <d3d11.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include "GameException.h"

using namespace Library;

D3DApp::D3DApp()
{
}


D3DApp::~D3DApp()
{
}

void D3DApp::Initialize()
{
	// create device
	HRESULT hr;
	UINT flags;
	D3D_FEATURE_LEVEL features[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1};
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceCtx;
	D3D_FEATURE_LEVEL featureLvl;


	if (FAILED(hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, features, ARRAYSIZE(features), D3D11_SDK_VERSION, device.GetAddressOf(), &featureLvl, deviceCtx.GetAddressOf())))
	{
		throw GameException("D3D11CreateDevice() failed", hr);
	}

	// check MSAA levels
	UINT numQlvls;
	if (FAILED(hr = device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &numQlvls)))
	{
		throw GameException("CheckMultisampleQualityLevels() failed", hr);
	}

	// create SwapChain
	DXGI_SAMPLE_DESC smplDesc;
	smplDesc.Count = 4;
	smplDesc.Quality = numQlvls;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	swapChainDesc.Width = 800; //todo
	swapChainDesc.Height = 600; //todo
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc = smplDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;


}
