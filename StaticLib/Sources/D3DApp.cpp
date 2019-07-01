#include "stdafx.h"
#include "D3DApp.h"

#include <d3d11.h>
#include <dxgi1_3.h>
#include <DirectXMath.h>
#include <wrl.h>


#include <DDSTextureLoader.h>

#include "RenderScene.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "FileManager.h"
#include "Mesh.h"
#include "Camera.h"
#include "GameException.h"

using namespace Library;

static Microsoft::WRL::ComPtr<ID3D11SamplerState> g_sampler;

void CreateSampler()
{
	// Create a sampler state for texture sampling in the pixel shader
	
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateSamplerState(&samplerDesc, g_sampler.GetAddressOf())))
	{
		throw GameException("CreateSamplerState() failed", hr);
	}
}



D3DApp::D3DApp(HWND hwnd, unsigned int width, unsigned int height) :
	m_hwnd(hwnd),
	m_width(width),
	m_height(height)
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
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 4;
	swapChainDesc.SampleDesc.Quality = numQlvls - 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

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
	dsbDesc.Width = m_width;
	dsbDesc.Height = m_height;
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

	// scissors test
	D3D11_RECT rects[1];
	rects[0].left = 0;
	rects[0].right = m_width;
	rects[0].top = 0;
	rects[0].bottom = m_height;

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

	// global
	m_globalApp.reset(new GD3DApp);
	g_D3D = m_globalApp.get();
	g_D3D->device = m_device.Get();
	g_D3D->deviceCtx = m_deviceCtx.Get();

	// shader mgr
	m_shaderManager.reset(new ShaderManager);
	m_shaderManager->Initialize();
	g_D3D->shaderMgr = m_shaderManager.get();

	// texture mgr
	m_textureManager.reset(new TextureManager);
	g_D3D->textureMgr = m_textureManager.get();

	// file mgr
	m_fileManager.reset(new FileManager);
	g_D3D->fileMgr = m_fileManager.get();

	// camera
	float fov = DirectX::XMConvertToRadians(45.f);
	m_camera.reset(new Camera(fov, m_width, m_height, 0.01f, 1000.0f));

	// scene
	m_renderScene.reset(new RenderScene);
	g_D3D->renderScene = m_renderScene.get();
	CreateSampler();
	//// todo: Mocking meshes
	//std::unique_ptr<Vertex[]> vertices(new Vertex[8]
	//	{
	//		Vertex(DirectX::XMFLOAT3(-1.0f, -1.0f, -1.0f), DirectX::XMFLOAT2(0.25f, 2.f / 3)),
	//		Vertex(DirectX::XMFLOAT3(-1.0f, 1.0f, -1.0f), DirectX::XMFLOAT2(0.25f, 1.f / 3)),
	//		Vertex(DirectX::XMFLOAT3(1.0f, 1.0f, -1.0f), DirectX::XMFLOAT2(0.5f, 1.f / 3)),
	//		Vertex(DirectX::XMFLOAT3(1.0f, -1.0f, -1.0f), DirectX::XMFLOAT2(0.5f, 2.f / 3)),
	//		Vertex(DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f), DirectX::XMFLOAT2(0.f, 2.f / 3)),
	//		Vertex(DirectX::XMFLOAT3(-1.0f, 1.0f, 1.0f), DirectX::XMFLOAT2(0.f, 1.f / 3)),
	//		Vertex(DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f), DirectX::XMFLOAT2(0.75f, 1.f / 3)),
	//		Vertex(DirectX::XMFLOAT3(1.0f, -1.0f, 1.0f), DirectX::XMFLOAT2(0.75f, 2.f / 3))
	//	});

	//std::unique_ptr<UINT[]> indices(new UINT[36]
	//	{
	//		0, 1, 2, 0, 2, 3,
	//		4, 6, 5, 4, 7, 6,
	//		4, 5, 1, 4, 1, 0,
	//		3, 2, 6, 3, 6, 7,
	//		1, 5, 6, 1, 6, 2,
	//		4, 0, 3, 4, 3, 7
	//	});

	//std::unique_ptr<Mesh> mesh(new Mesh(std::move(vertices), 8, std::move(indices), 36, "crate_diffuse.dds"));
	//m_renderScene->AddMesh(std::move(mesh));
}


void D3DApp::Draw(const GameTime &gameTime) 
{
	// clear rt
	const DirectX::XMVECTORF32 BackgroundColor = { 0.392f,0.584f, 0.929f, 1.0f };
	m_deviceCtx->ClearRenderTargetView(m_rtv.Get(), reinterpret_cast<const float*>(&BackgroundColor));
	m_deviceCtx->ClearDepthStencilView(m_dsbView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// set views to OM stage
	m_deviceCtx->OMSetRenderTargets(1, m_rtv.GetAddressOf(), m_dsbView.Get());

	// IA
	for (auto it = m_renderScene->BeginMesh(); it != m_renderScene->EndMesh(); ++it)
	{
		DrawMesh((*it).get());
		//break;
	}

	// present
	HRESULT hr = m_swapChain->Present(0, 0);
	if (FAILED(hr))
	{
		throw GameException("IDXGISwapChain::Present() failed.", hr);
	}
}

void Library::D3DApp::DrawMesh(Mesh* mesh)
{
	// update mesh cb
	MeshCB meshCb;
	DirectX::XMStoreFloat4x4(&meshCb.WorldViewProj, *(mesh->GetModelTransform()) * *(m_camera->GetView()) * *(m_camera->GetProjection()));
	meshCb.AmbientK = 0.9;
	meshCb.EmissiveK = 0.5;
	meshCb.DiffuseIntensity = 0.9;
	meshCb.Roughness = 0.5;

	m_deviceCtx->UpdateSubresource(mesh->GetConstMeshBuffer(), 0, nullptr, &meshCb, 0, 0);
	
	// update scene CB

	DirectX::XMFLOAT3 eyePos; // todo: Get eyePos from camera
	m_deviceCtx->UpdateSubresource(m_renderScene->GetConstSceneBuffer(), 0, NULL, &eyePos, 0, 0);

	// IA
	UINT stride = sizeof(Library::Vertex);
	UINT offset = 0;
	m_deviceCtx->IASetVertexBuffers(0, 1, mesh->GetVertexBufferRef(), &stride, &offset);
	m_deviceCtx->IASetIndexBuffer(mesh->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);
	m_deviceCtx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_deviceCtx->IASetInputLayout(mesh->GetInputLayout());

	// VS
	ID3D11VertexShader* vs = m_shaderManager->GetVertexShader(mesh->GetVertexShader());
	m_deviceCtx->VSSetShader(vs, NULL, 0); //todo: mocking: ASSUMPTION: we use same vs for each
	m_deviceCtx->VSSetConstantBuffers(0, 1, mesh->GetConstMeshBufferRef());
	m_deviceCtx->VSSetConstantBuffers(1, 1, m_renderScene->GetConstSceneBufferRef());

	// PS
	ID3D11PixelShader* ps = m_shaderManager->GetPixelShader(mesh->GetPixelShader());
	m_deviceCtx->PSSetShader(ps, NULL, 0);
	m_deviceCtx->PSSetSamplers(0, 1, g_sampler.GetAddressOf());
	m_deviceCtx->PSSetShaderResources(0, 1, g_D3D->textureMgr->GetTexture(mesh->GetDiffuseTexture()));

	// draw call
	m_deviceCtx->DrawIndexed(mesh->GetIndexCount(), 0, 0);
}
