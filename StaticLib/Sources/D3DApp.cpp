#include "stdafx.h"
#include "D3DApp.h"

#include <d3d11.h>
#include <dxgi1_3.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <d3dcompiler.h>

#include "GameException.h"

using namespace Library;

D3DApp::D3DApp(HWND hwnd) : 
	m_hwnd(hwnd), 
	m_viewport(new D3D11_VIEWPORT)
{
}


D3DApp::~D3DApp()
{
}

void D3DApp::Initialize()
{
	// create device
	HRESULT hr;
	UINT flags(0);
#if defined(_DEBUG)
	// If the project is in a debug build, enable the debug layer.
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL features[] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_11_1};
	D3D_FEATURE_LEVEL featureLvl;

	if (FAILED(hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, flags, features, ARRAYSIZE(features), D3D11_SDK_VERSION, m_device.GetAddressOf(), &featureLvl, m_deviceCtx.GetAddressOf())))
	{
		throw GameException("D3D11CreateDevice() failed", hr);
	}

	// check MSAA levels
	UINT numQlvls;
	if (FAILED(hr = m_device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, 4, &numQlvls)))
	{
		throw GameException("CheckMultisampleQualityLevels() failed", hr);
	}

	// create SwapChain
	if (FAILED(hr = CreateDXGIFactory2(0, IID_PPV_ARGS(m_factory.GetAddressOf()))))
	{
		throw GameException("CreateDXGIFactory2() failed", hr);
	}

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.Width = 800; //todo
	swapChainDesc.Height = 600; //todo
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 4;
	swapChainDesc.SampleDesc.Quality = numQlvls - 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc;
	ZeroMemory(&fullScreenDesc, sizeof(fullScreenDesc));
	fullScreenDesc.RefreshRate.Numerator = 120;
	fullScreenDesc.RefreshRate.Denominator = 1;
	fullScreenDesc.Windowed = false;

	IDXGIDevice* dxgiDevice = nullptr;
	if (FAILED(hr = m_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice))))
	{
		throw GameException("ID3D11Device::QueryInterface() failed", hr);
	}

	IDXGIAdapter *dxgiAdapter = nullptr;
	if (FAILED(hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgiAdapter))))
	{
		throw GameException("IDXGIDevice::GetParent() failed retrieving adapter.", hr);
	}

	IDXGIFactory2* dxgiFactory = nullptr;
	if (FAILED(hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory))))
	{
		throw GameException("IDXGIAdapter::GetParent() failed retrieving factory.", hr);
	}

	if (FAILED(hr = dxgiFactory->CreateSwapChainForHwnd(dxgiDevice, m_hwnd, &swapChainDesc, NULL, NULL, m_swapChain.GetAddressOf())))
	{
		throw GameException("CreateSwapChainForHwnd() failed", hr);
	}

	// create RTV
	Microsoft::WRL::ComPtr<ID3D11Texture2D> backbuffer;
	if (FAILED(hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(backbuffer.GetAddressOf()))))
	{
		throw GameException("GetBuffer() failed", hr);
	}

	if (FAILED(hr = m_device->CreateRenderTargetView(backbuffer.Get(), NULL, m_rtv.GetAddressOf())))
	{
		throw GameException("CreateRenderTargetView() failed", hr);
	}

	// create DSB
	Microsoft::WRL::ComPtr<ID3D11Texture2D> dsb;
	D3D11_TEXTURE2D_DESC dsbDesc;
	ZeroMemory(&dsbDesc, sizeof(dsbDesc));
	dsbDesc.Width = 800; // todo
	dsbDesc.Height = 600; // todo
	dsbDesc.MipLevels = 1;
	dsbDesc.ArraySize = 1;
	dsbDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsbDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dsbDesc.Usage = D3D11_USAGE_DEFAULT;
	dsbDesc.SampleDesc.Count = 4;
	dsbDesc.SampleDesc.Quality = numQlvls - 1;

	if (FAILED(hr = m_device->CreateTexture2D(&dsbDesc, NULL, dsb.GetAddressOf())))
	{
		throw GameException("CreateTexture2D() failed", hr);
	}

	if (FAILED(hr = m_device->CreateDepthStencilView(dsb.Get(), NULL, m_dsbView.GetAddressOf())))
	{
		throw GameException("CreateDepthStencilView() failed", hr);
	}

	// set views to OM stage
	m_deviceCtx->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_dsbView.Get());

	//set viewport
	m_viewport->Width = 800.f; //todo
	m_viewport->Height = 600.f; //todo
	m_viewport->TopLeftX = 0.f;
	m_viewport->TopLeftY = 0.f;
	m_viewport->MinDepth = 0.f;
	m_viewport->MaxDepth = 1.f;
	
	m_deviceCtx->RSSetViewports(1, m_viewport.get());

	// setup shaders
	// vertex
	Microsoft::WRL::ComPtr<ID3DBlob> VS_Buffer, PS_Buffer;
	if (FAILED(hr = D3DReadFileToBlob(L"../Bin/x64/Debug/VertexShader.cso", &VS_Buffer)))
	{
		throw GameException("D3DReadFileToBlob() for VS failed", hr);
	}
	if (FAILED(hr = m_device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, m_vertexShader.GetAddressOf())))
	{
		throw GameException("CreateVertexShader() failed", hr);
	}

	m_deviceCtx->VSSetShader(m_vertexShader.Get(), NULL, 0);


	// pixel
	if (FAILED(hr = D3DReadFileToBlob(L"../Bin/x64/Debug/PixelShader.cso", &PS_Buffer)))
	{
		throw GameException("D3DReadFileToBlob() for PS failed", hr);
	}
	if (FAILED(hr = m_device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf())))
	{
		throw GameException("CreateVertexShader() failed", hr);
	}

	m_deviceCtx->PSSetShader(m_pixelShader.Get(), NULL, 0);

	// setup IA layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	if (FAILED(hr = m_device->CreateInputLayout(layout, 1, VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), m_Inputlayout.GetAddressOf())))
	{
		throw GameException("CreateInputLayout() failed", hr);
	}

	// setup constant buffer
	struct VS_CONSTANT_BUFFER
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
	};

	VS_CONSTANT_BUFFER VsConstData;
	VsConstData.WorldViewProj = DirectX::XMFLOAT4X4();

	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(VS_CONSTANT_BUFFER);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &VsConstData;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	if (FAILED(hr = m_device->CreateBuffer(&cbDesc, &InitData, &m_constBuffer)))
	{
		throw GameException("CreateBuffer() failed", hr);
	}

	m_deviceCtx->VSSetConstantBuffers(0, 1, m_constBuffer.GetAddressOf());
}


void D3DApp::Draw(const GameTime &gameTime) 
{
	// clear rt
	const DirectX::XMVECTORF32 BackgroundColor = { 0.392f,0.584f, 0.929f, 1.0f };
	m_deviceCtx->ClearRenderTargetView(m_rtv.Get(), reinterpret_cast<const float*>(&BackgroundColor));
	m_deviceCtx->ClearDepthStencilView(m_dsbView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// IA

	// debug
	DirectX::XMFLOAT3 Vbuff[] = {
		{1.f, 1.f, 1.f},
		{0.f, 0.f, 0.f},
		{-1.f, -1.f, -1.f}
	};

	UINT stride = sizeof(DirectX::XMFLOAT3);
	UINT offset = 0;

	// Fill in a buffer description.
	D3D11_BUFFER_DESC bufferDesc;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT3) * 3;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;

	// Fill in the sub-resource data.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = Vbuff;
	InitData.SysMemPitch = 0;
	InitData.SysMemSlicePitch = 0;

	// Create the vertex buffer.
	HRESULT hr;
	if (FAILED(hr = m_device->CreateBuffer(&bufferDesc, &InitData, &m_vertexBuffer)))
	{
		throw GameException("CreateBuffer() failed", hr);
	}

	// end debug
	m_deviceCtx->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	m_deviceCtx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceCtx->IASetInputLayout(m_Inputlayout.Get());

	// draw call
	m_deviceCtx->Draw(1, 0);

	// present
	hr = m_swapChain->Present(0, 0);
	if (FAILED(hr)) 
	{ 
		throw GameException("IDXGISwapChain::Present() failed.", hr); 
	}
}