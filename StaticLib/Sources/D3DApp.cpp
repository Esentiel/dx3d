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
#include "PostProcessor.h"
#include "ShadowMap.h"

using namespace Library;

static Microsoft::WRL::ComPtr<ID3D11SamplerState> s_sampler;
static Microsoft::WRL::ComPtr<ID3D11SamplerState> s_sampler2;

void CreateSampler(ID3D11SamplerState **sampler)
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
	if (FAILED(hr = g_D3D->device->CreateSamplerState(&samplerDesc, sampler)))
	{
		THROW_GAME_EXCEPTION("CreateSamplerState() failed", hr);
	}
}

void CreateSampler2(ID3D11SamplerState **sampler)
{
	// Create a sampler state for texture sampling in the pixel shader

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
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
	if (FAILED(hr = g_D3D->device->CreateSamplerState(&samplerDesc, sampler)))
	{
		THROW_GAME_EXCEPTION("CreateSamplerState() failed", hr);
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
		THROW_GAME_EXCEPTION("D3D11CreateDevice() failed", hr);
	}

	// check MSAA levels
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	for (m_sampleCount = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
		m_sampleCount > 1; m_sampleCount--)
	{
		if (FAILED(m_device->CheckMultisampleQualityLevels(format, m_sampleCount, &m_levels)))
			continue;

		if (m_levels > 0)
			break;
	}

	// create SwapChain
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = format;
	swapChainDesc.SampleDesc.Count = m_sampleCount;
	swapChainDesc.SampleDesc.Quality = m_levels - 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 3;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

	IDXGIDevice* dxgiDevice = nullptr;
	if (FAILED(hr = m_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice))))
	{
		THROW_GAME_EXCEPTION("ID3D11Device::QueryInterface() failed", hr);
	}

	IDXGIAdapter *dxgiAdapter = nullptr;
	if (FAILED(hr = dxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgiAdapter))))
	{
		THROW_GAME_EXCEPTION("IDXGIDevice::GetParent() failed retrieving adapter.", hr);
	}

	IDXGIFactory2* dxgiFactory = nullptr;
	if (FAILED(hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory))))
	{
		THROW_GAME_EXCEPTION("IDXGIAdapter::GetParent() failed retrieving factory.", hr);
	}

	if (FAILED(hr = dxgiFactory->CreateSwapChainForHwnd(dxgiDevice, m_hwnd, &swapChainDesc, NULL, NULL, m_swapChain.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("CreateSwapChainForHwnd() failed", hr);
	}

	// create RTV
	m_backBuffers.resize(1);
	if (FAILED(hr = m_swapChain->GetBuffer(0, IID_PPV_ARGS(m_backBuffers.at(0).GetAddressOf()))))
	{
		THROW_GAME_EXCEPTION("GetBuffer() failed", hr);
	}
	m_rtvs.resize(3);
	CD3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc(D3D11_RTV_DIMENSION_TEXTURE2DMS);
	if (FAILED(hr = m_device->CreateRenderTargetView(m_backBuffers.at(0).Get(), &renderTargetViewDesc, m_rtvs.at(0).GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("CreateRenderTargetView() failed", hr);
	}

	// 2
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = m_width;
	desc.Height = m_height;
	desc.Format = format;
	desc.SampleDesc.Count = m_sampleCount;
	desc.SampleDesc.Quality = m_levels - 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_RENDER_TARGET;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture = NULL;
	if (FAILED(hr = m_device->CreateTexture2D(&desc, NULL, &pTexture)))
	{
		THROW_GAME_EXCEPTION("CreateTexture2D() failed for 2nd RTV", hr);
	}

	m_backBuffers.push_back(pTexture);
	pTexture.Reset();

	if (FAILED(hr = m_device->CreateRenderTargetView(m_backBuffers.at(1).Get(), &renderTargetViewDesc, m_rtvs.at(1).GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("CreateRenderTargetView() failed for 2nd RTV", hr);
	}

	// 3
	if (FAILED(hr = m_device->CreateTexture2D(&desc, NULL, &pTexture)))
	{
		THROW_GAME_EXCEPTION("CreateTexture2D() failed for 3rd RTV", hr);
	}

	m_backBuffers.push_back(pTexture);
	if (FAILED(hr = m_device->CreateRenderTargetView(m_backBuffers.at(2).Get(), &renderTargetViewDesc, m_rtvs.at(2).GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("CreateRenderTargetView() failed for 3rd RTV", hr);
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
	dsbDesc.SampleDesc.Count =  m_sampleCount;
	dsbDesc.SampleDesc.Quality = m_levels - 1;

	if (FAILED(hr = m_device->CreateTexture2D(&dsbDesc, NULL, dsb.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("CreateTexture2D() failed", hr);
	}

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2DMS);
	if (FAILED(hr = m_device->CreateDepthStencilView(dsb.Get(), &depthStencilViewDesc, m_dsbView.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("CreateDepthStencilView() failed", hr);
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
	rasterizerState.MultisampleEnable = true;
	rasterizerState.AntialiasedLineEnable = false;
	m_device->CreateRasterizerState(&rasterizerState, &m_rasterState);

	// blend
	

	D3D11_BLEND_DESC BlendState;
	ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC));
	BlendState.RenderTarget[0].BlendEnable = FALSE;
	BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	m_device->CreateBlendState(&BlendState, m_blendStateNoBlend.GetAddressOf());

	// device
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
	m_camera.reset(new Camera(fov, m_width, m_height, 0.01f, 300.0f));
	g_D3D->camera = m_camera.get();

	// scene
	m_renderScene.reset(new RenderScene);
	g_D3D->renderScene = m_renderScene.get();
	CreateSampler(s_sampler.GetAddressOf());
	CreateSampler2(s_sampler2.GetAddressOf());

	// Post-processor
	m_postProcessor.reset(new PostProcessor(m_width, m_height));
	m_postProcessor->Initialize();

	// ShadowMap
	m_shadowMap.reset(new ShadowMap);	
}


void D3DApp::Draw(const GameTime& g) 
{
	//m_renderScene->Update(g.ElapsedGameTime());

	SceneCB sceneCb;
	DirectX::XMFLOAT3 camPos;
	DirectX::XMStoreFloat3(&camPos, m_camera->GetPosition());
	sceneCb.EyePos = DirectX::XMFLOAT4(camPos.x, camPos.y, camPos.z, 1.0f);
	sceneCb.GlobalAmbient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
	for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; i++)
	{
		sceneCb.Lights[i] = m_renderScene->GetSceneLights()[i];
	}

	// Generate shadow map
	m_shadowMap->Initialize(m_width, m_height);
	m_shadowMap->SetLightSource(sceneCb.Lights);
	m_shadowMap->Generate(m_renderScene.get());
	m_camera->UpdateViewport();

	// begin Post processing
	m_postProcessor->Begin();

	// update scene CB
	m_deviceCtx->UpdateSubresource(m_renderScene->GetConstSceneBuffer(), 0, NULL, &sceneCb, 0, 0);

	// skyBox
	m_renderScene->DrawSkyBox();

	// raster state
	m_deviceCtx->RSSetState(m_rasterState.Get());

	// blend
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	UINT sampleMask = 0xffffffff;

	m_deviceCtx->OMSetBlendState(m_blendStateNoBlend.Get(), blendFactor, sampleMask);
	// meshes
	m_deviceCtx->PSSetShaderResources(4, MAX_LIGHT_SOURCES, m_shadowMap->GetShadowMapRef());
	
	for (auto it = m_renderScene->BeginMesh(); it != m_renderScene->EndMesh(); ++it)
	{
		DrawMesh((*it).get());
	}
	
	// set views to OM stage
	m_deviceCtx->OMSetRenderTargets(1, m_rtvs.at(0).GetAddressOf(), m_dsbView.Get());

	// clear rt
	const DirectX::XMVECTORF32 BackgroundColor = { 0.392f,0.584f, 0.929f, 1.0f };
	m_deviceCtx->ClearRenderTargetView(m_rtvs.at(0).Get(), reinterpret_cast<const float*>(&BackgroundColor));
	m_deviceCtx->ClearDepthStencilView(m_dsbView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// draw offscreen texture to backbuffer
	m_postProcessor->Draw();

	// present
	HRESULT hr = m_swapChain->Present(0, 0);
	if (FAILED(hr))
	{
		THROW_GAME_EXCEPTION("IDXGISwapChain::Present() failed.", hr);
	}
}

void Library::D3DApp::DrawMesh(Mesh* mesh)
{
	ID3D11ShaderResourceView *const pSRV[1] = { NULL };

	// update mesh cb
	DirectX::XMFLOAT4X4 mProjectedTextureScalingMatrix;
	mProjectedTextureScalingMatrix._11 = 0.5f;
	mProjectedTextureScalingMatrix._12 = 0.f;
	mProjectedTextureScalingMatrix._13 = 0.f;
	mProjectedTextureScalingMatrix._14 = 0.f;
	mProjectedTextureScalingMatrix._21 = 0.0f;
	mProjectedTextureScalingMatrix._22 = -0.5f;
	mProjectedTextureScalingMatrix._23 = 0.0f;
	mProjectedTextureScalingMatrix._24 = 0.0f;
	mProjectedTextureScalingMatrix._31 = 0.0f;
	mProjectedTextureScalingMatrix._32 = 0.0f;
	mProjectedTextureScalingMatrix._33 = 1.0f;
	mProjectedTextureScalingMatrix._34 = 0.0f;
	mProjectedTextureScalingMatrix._41 = 0.5f;
	mProjectedTextureScalingMatrix._42 = 0.5f;
	mProjectedTextureScalingMatrix._43 = 0.0f;
	mProjectedTextureScalingMatrix._44 = 1.0f;

	MeshCB meshCb;
	DirectX::XMStoreFloat4x4(&meshCb.WorldViewProj, *(mesh->GetModelTransform()) * m_camera->GetView() * m_camera->GetProjection());
	DirectX::XMStoreFloat4x4(&meshCb.World, *(mesh->GetModelTransform()));
	
	meshCb.MaterialInstance.Emissive = DirectX::XMFLOAT4(0.f, 0.f, 0.f, 0.f);
	meshCb.MaterialInstance.Ambient = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.f);
	meshCb.MaterialInstance.Diffuse = DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	meshCb.MaterialInstance.Specular = DirectX::XMFLOAT4(1.f, 1.f, 1.f, 1.f);
	meshCb.MaterialInstance.SpecularPower = 32;
	meshCb.MaterialInstance.HasNormalMap = mesh->GetFlag(Mesh::MeshFlags::UseNormalMap); // todo: remove this, use GetSize() on texture in shader
	meshCb.MaterialInstance.HasSpecularMap = mesh->GetFlag(Mesh::MeshFlags::UseSpecularMap); // todo: remove this, use GetSize() on texture in shader
	meshCb.MaterialInstance.roughness = mesh->GetFlag(Mesh::MeshFlags::UseSpecularReflection) ? 0.3f : 1.0f;

	m_deviceCtx->UpdateSubresource(mesh->GetConstMeshBuffer(), 0, nullptr, &meshCb, 0, 0);

	ShadowMap::SMCB meshSM;
	for (unsigned int i = 0; i < MAX_LIGHT_SOURCES; i++)
	{
		DirectX::XMStoreFloat4x4(&(meshSM.shadowMapVP[i]), *(mesh->GetModelTransform()) * m_shadowMap->GetViewMatrix(i) * m_shadowMap->GetProjection() * DirectX::XMLoadFloat4x4(&mProjectedTextureScalingMatrix));
	}
	m_deviceCtx->UpdateSubresource(m_shadowMap->GetConstMeshBuffer(), 0, nullptr, &meshSM, 0, 0);

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
	m_deviceCtx->VSSetConstantBuffers(2, 1, m_shadowMap->GetConstMeshBufferRef());

	// PS
	ID3D11PixelShader* ps = m_shaderManager->GetPixelShader(mesh->GetPixelShader());
	m_deviceCtx->PSSetShader(ps, NULL, 0);
	m_deviceCtx->PSSetSamplers(0, 1, s_sampler.GetAddressOf());
	m_deviceCtx->PSSetSamplers(1, 1, s_sampler2.GetAddressOf());
	m_deviceCtx->PSSetShaderResources(0, 1, g_D3D->textureMgr->GetTexture(mesh->GetTexturePath(Mesh::TextureType::DiffuseTexture)));
	if (meshCb.MaterialInstance.HasNormalMap)
		m_deviceCtx->PSSetShaderResources(2, 1, g_D3D->textureMgr->GetTexture(mesh->GetTexturePath(Mesh::TextureType::NormalTexture)));
	else
		g_D3D->deviceCtx->PSSetShaderResources(2, 1, pSRV);
	if (meshCb.MaterialInstance.HasSpecularMap)
		m_deviceCtx->PSSetShaderResources(3, 1, g_D3D->textureMgr->GetTexture(mesh->GetTexturePath(Mesh::TextureType::SpecularTexture)));
	else
		g_D3D->deviceCtx->PSSetShaderResources(3, 1, pSRV);
	m_deviceCtx->PSSetConstantBuffers(0, 1, mesh->GetConstMeshBufferRef());
	m_deviceCtx->PSSetConstantBuffers(1, 1, m_renderScene->GetConstSceneBufferRef());

	// draw call
	m_deviceCtx->DrawIndexed(mesh->GetIndexCount(), 0, 0);
}
