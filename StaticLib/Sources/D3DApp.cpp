#include "stdafx.h"
#include "D3DApp.h"

#include <d3d11.h>
#include <dxgi1_3.h>
#include <DirectXMath.h>
#include <wrl.h>
#include <d3dcompiler.h>

#include "GameException.h"

using namespace Library;

DirectX::XMFLOAT4X4 g_mvp;

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

	//set viewport
	m_viewport->Width = 800.f; //todo
	m_viewport->Height = 600.f; //todo
	m_viewport->TopLeftX = 0.f;
	m_viewport->TopLeftY = 0.f;
	m_viewport->MinDepth = 0.f;
	m_viewport->MaxDepth = 1.f;
	
	m_deviceCtx->RSSetViewports(1, m_viewport.get());

	// load shaders
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

	// pixel
	if (FAILED(hr = D3DReadFileToBlob(L"../Bin/x64/Debug/PixelShader.cso", &PS_Buffer)))
	{
		throw GameException("D3DReadFileToBlob() for PS failed", hr);
	}
	if (FAILED(hr = m_device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), nullptr, m_pixelShader.GetAddressOf())))
	{
		throw GameException("CreateVertexShader() failed", hr);
	}

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

	CD3D11_BUFFER_DESC cbDesc(
		sizeof(VsConstData),
		D3D11_BIND_CONSTANT_BUFFER
	);

	if (FAILED(hr = m_device->CreateBuffer(&cbDesc, NULL, &m_constBuffer)))
	{
		throw GameException("CreateBuffer() failed", hr);
	}

	// scissors test
	D3D11_RECT rects[1];
	rects[0].left = 0;
	rects[0].right = 800;
	rects[0].top = 0;
	rects[0].bottom = 600;

	m_deviceCtx->RSSetScissorRects(1, rects);

	// rasterizer
	D3D11_RASTERIZER_DESC rasterizerState;
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.CullMode = D3D11_CULL_BACK;
	rasterizerState.FrontCounterClockwise = true;
	rasterizerState.DepthBias = false;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.ScissorEnable = true;
	rasterizerState.MultisampleEnable = false;
	rasterizerState.AntialiasedLineEnable = false;
	m_device->CreateRasterizerState(&rasterizerState, &m_rasterState);

	m_deviceCtx->RSSetState(m_rasterState.Get());

	// debug cube
	CreateCube();
}


void D3DApp::Draw(const GameTime &gameTime) 
{
	// debug cube
	DirectX::XMFLOAT4X4 mvp;
	DirectX::XMMATRIX model, view, projection;

	float angle = 90.0f;
	const DirectX::XMVECTOR rotationAxis = DirectX::XMVectorSet(0, 1, 1, 0);
	model = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));

	DirectX::XMVECTOR eye = DirectX::XMVectorSet(0.0f, 0.f, 10.0f, 1.0f);
	DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.0f, -1.0f, 1.0f);
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	view = DirectX::XMMatrixLookToRH(eye, at, up);

	float aspectRatio = (float)800/600; // todo
	auto fov = DirectX::XMConvertToRadians(45.0f);

	projection = DirectX::XMMatrixPerspectiveFovRH(fov, aspectRatio, 0.01f, 1000.0f);

	DirectX::XMStoreFloat4x4(&mvp, model * view * projection);

	m_deviceCtx->UpdateSubresource(m_constBuffer.Get(),	0, nullptr,	&mvp, 0, 0);

	// clear rt
	const DirectX::XMVECTORF32 BackgroundColor = { 0.392f,0.584f, 0.929f, 1.0f };
	m_deviceCtx->ClearRenderTargetView(m_rtv.Get(), reinterpret_cast<const float*>(&BackgroundColor));
	m_deviceCtx->ClearDepthStencilView(m_dsbView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// IA
	UINT stride = sizeof(DirectX::XMFLOAT3);
	UINT offset = 0;
	m_deviceCtx->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	m_deviceCtx->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_deviceCtx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceCtx->IASetInputLayout(m_Inputlayout.Get());

	// VS
	m_deviceCtx->VSSetShader(m_vertexShader.Get(), NULL, 0);
	m_deviceCtx->VSSetConstantBuffers(0, 1,	m_constBuffer.GetAddressOf());

	// PS
	m_deviceCtx->PSSetShader(m_pixelShader.Get(), NULL, 0);

	// draw call
	m_deviceCtx->DrawIndexed(36, 0, 0);

	// set views to OM stage
	m_deviceCtx->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_dsbView.Get());

	// present
	HRESULT hr = m_swapChain->Present(0, 0);
	if (FAILED(hr))
	{
		throw GameException("IDXGISwapChain::Present() failed.", hr);
	}
}

void Library::D3DApp::CreateCube()
{
	DirectX::XMFLOAT3 CubeVertices[] =
	{
	 DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), // 0
	 DirectX::XMFLOAT3(-1.0f,  1.0f, -1.0f), // 1
	 DirectX::XMFLOAT3(1.0f,  1.0f, -1.0f), // 2
	 DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), // 3
	 DirectX::XMFLOAT3(-1.0f, -1.0f,  1.0f), // 4
	 DirectX::XMFLOAT3(-1.0f,  1.0f,  1.0f), // 5
	 DirectX::XMFLOAT3(1.0f,  1.0f,  1.0f), // 6
	 DirectX::XMFLOAT3(1.0f, -1.0f,  1.0f)  // 7
	};

	CD3D11_BUFFER_DESC vDesc(
		sizeof(CubeVertices),
		D3D11_BIND_VERTEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA vData;
	ZeroMemory(&vData, sizeof(D3D11_SUBRESOURCE_DATA));
	vData.pSysMem = CubeVertices;
	vData.SysMemPitch = 0;
	vData.SysMemSlicePitch = 0;

	HRESULT hr = m_device->CreateBuffer(&vDesc, &vData,	m_vertexBuffer.GetAddressOf());

	UINT CubeIndices[] =
	{
	0, 1, 2, 0, 2, 3,
	4, 6, 5, 4, 7, 6,
	4, 5, 1, 4, 1, 0,
	3, 2, 6, 3, 6, 7,
	1, 5, 6, 1, 6, 2,
	4, 0, 3, 4, 3, 7
	};


	m_indexCount = ARRAYSIZE(CubeIndices);

	CD3D11_BUFFER_DESC iDesc(
		sizeof(CubeIndices),
		D3D11_BIND_INDEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA iData;
	ZeroMemory(&iData, sizeof(D3D11_SUBRESOURCE_DATA));
	iData.pSysMem = CubeIndices;
	iData.SysMemPitch = 0;
	iData.SysMemSlicePitch = 0;

	hr = m_device->CreateBuffer(&iDesc,	&iData,	m_indexBuffer.GetAddressOf());
}
