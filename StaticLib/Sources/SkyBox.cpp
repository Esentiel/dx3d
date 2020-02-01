#include "stdafx.h"
#include "SkyBox.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include "TextureManager.h"
#include "Mesh.h"
#include "Transformations.h"
#include "GameException.h"
#include "RenderScene.h"
#include "Camera.h"
#include "ShaderManager.h"

using namespace Library;

SkyBox::SkyBox() :
	m_transformations(std::make_unique<Transformations>())
{
	CreateSkySphere();
	CreateRasterState();
	CreateDSState();
	CreateSamplerState();
	CreateTexture();
	CreateInputLayout();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateConstMeshBuffer();

	m_transformations->Scale(DirectX::XMFLOAT3(500.f, 500.f, 500.f));
}


SkyBox::~SkyBox()
{
}

void SkyBox::CreateRasterState()
{
	// rasterizer
	D3D11_RASTERIZER_DESC rasterizerState;
	ZeroMemory(&rasterizerState, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.CullMode = D3D11_CULL_NONE;
	rasterizerState.DepthBias = 0;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0.0f;
	rasterizerState.DepthClipEnable = true;
	rasterizerState.ScissorEnable = true;
	rasterizerState.MultisampleEnable = false;
	rasterizerState.AntialiasedLineEnable = false;
	g_D3D->device->CreateRasterizerState(&rasterizerState, &m_rasterState);
}

void Library::SkyBox::Draw(RenderScene * renderScene) // TODO: rewrite to use Mesh
{
	// change raster state
	g_D3D->deviceCtx->RSSetState(m_rasterState.Get());

	// draw sphere
	// update mesh cb
	MeshCB meshCb;
	DirectX::XMStoreFloat4x4(&meshCb.Model, m_transformations->GetModel());
	DirectX::XMStoreFloat4x4(&meshCb.View, g_D3D->camera->GetView());
	DirectX::XMStoreFloat4x4(&meshCb.Projection, g_D3D->camera->GetProjection());

	g_D3D->deviceCtx->UpdateSubresource(m_constMeshBuffer.Get(), 0, nullptr, &meshCb, 0, 0);

	// IA
	UINT stride = sizeof(DirectX::XMFLOAT3);
	UINT offset = 0;
	g_D3D->deviceCtx->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
	g_D3D->deviceCtx->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	g_D3D->deviceCtx->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	g_D3D->deviceCtx->IASetInputLayout(m_inputlayout.Get());

	// VS
	ID3D11VertexShader* vs = g_D3D->shaderMgr->GetVertexShader(m_VSName);
	g_D3D->deviceCtx->VSSetShader(vs, NULL, 0);
	g_D3D->deviceCtx->VSSetConstantBuffers(0, 1, m_constMeshBuffer.GetAddressOf());
	g_D3D->deviceCtx->VSSetConstantBuffers(1, 1, renderScene->GetConstSceneBufferRef());

	// PS
	ID3D11PixelShader* ps = g_D3D->shaderMgr->GetPixelShader(m_PSName);
	g_D3D->deviceCtx->PSSetShader(ps, NULL, 0);
	g_D3D->deviceCtx->PSSetSamplers(2, 1, m_cubesTexSamplerState.GetAddressOf());
	g_D3D->deviceCtx->PSSetShaderResources(1, 1, m_cubesTextureRes.GetAddressOf());

	g_D3D->deviceCtx->OMSetDepthStencilState(m_dsv.Get(), 0);

	// draw call
	g_D3D->deviceCtx->DrawIndexed((UINT)m_indices.size(), 0, 0);

	g_D3D->deviceCtx->OMSetDepthStencilState(0, 0);
}

void Library::SkyBox::CreateSkySphere()
{
	//
	// Compute the vertices stating at the top pole and moving down the stacks.
	//

	// Poles: note that there will be texture coordinate distortion as there is
	// not a unique point on the texture map to assign to the pole when mapping
	// a rectangular texture onto a sphere.

	float radius = 0.5f;
	int stackCount = 20;
	int sliceCount = 20;

	DirectX::XMFLOAT3 topVertex(0.0f, +radius, 0.0f);
	DirectX::XMFLOAT3 bottomVertex(0.0f, -radius, 0.0f);

	m_vertices.push_back(topVertex);

	float phiStep = DirectX::XM_PI / stackCount;
	float thetaStep = 2.0f * DirectX::XM_PI / sliceCount;

	// Compute vertices for each stack ring (do not count the poles as rings).
	for (int i = 1; i <= stackCount - 1; ++i)
	{
		float phi = i * phiStep;

		// Vertices of ring.
		for (int j = 0; j <= sliceCount; ++j)
		{
			float theta = j * thetaStep;

			DirectX::XMFLOAT3 v;

			// spherical to cartesian
			v.x = radius * sinf(phi)*cosf(theta);
			v.y = radius * cosf(phi);
			v.z = radius * sinf(phi)*sinf(theta);

			m_vertices.push_back(v);
		}
	}

	m_vertices.push_back(bottomVertex);

	//
	// Compute indices for top stack.  The top stack was written first to the vertex buffer
	// and connects the top pole to the first ring.
	//

	for (int i = 1; i <= sliceCount; ++i)
	{
		m_indices.push_back(0);
		m_indices.push_back(i + 1);
		m_indices.push_back(i);
	}

	//
	// Compute indices for inner stacks (not connected to poles).
	//

	// Offset the indices to the index of the first vertex in the first ring.
	// This is just skipping the top pole vertex.
	int baseIndex = 1;
	int ringVertexCount = sliceCount + 1;
	for (int i = 0; i < stackCount - 2; ++i)
	{
		for (int j = 0; j < sliceCount; ++j)
		{
			m_indices.push_back(baseIndex + i * ringVertexCount + j);
			m_indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			m_indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);

			m_indices.push_back(baseIndex + (i + 1)*ringVertexCount + j);
			m_indices.push_back(baseIndex + i * ringVertexCount + j + 1);
			m_indices.push_back(baseIndex + (i + 1)*ringVertexCount + j + 1);
		}
	}

	//
	// Compute indices for bottom stack.  The bottom stack was written last to the vertex buffer
	// and connects the bottom pole to the bottom ring.
	//

	// South pole vertex was added last.
	int southPoleIndex = (int)m_indices.size() - 1;

	// Offset the indices to the index of the first vertex in the last ring.
	baseIndex = southPoleIndex - ringVertexCount;

	for (int i = 0; i < sliceCount; ++i)
	{
		m_indices.push_back(southPoleIndex);
		m_indices.push_back(baseIndex + i);
		m_indices.push_back(baseIndex + i + 1);
	}
}

void Library::SkyBox::CreateDSState()
{
	D3D11_DEPTH_STENCIL_DESC dssDesc;
	ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	dssDesc.DepthEnable = true;
	dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	g_D3D->device->CreateDepthStencilState(&dssDesc, m_dsv.GetAddressOf());
}

void Library::SkyBox::CreateSamplerState()
{
	// Describe the Sample State
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//Create the Sample State
	g_D3D->device->CreateSamplerState(&sampDesc, m_cubesTexSamplerState.GetAddressOf());
}

void Library::SkyBox::CreateTexture()
{
	Microsoft::WRL::ComPtr<ID3D11Texture2D> boxTexture;
	g_D3D->textureMgr->LoadTexture(m_textureName, boxTexture.GetAddressOf());

	D3D11_TEXTURE2D_DESC  boxTextureDesc;
	boxTexture->GetDesc(&boxTextureDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC boxViewDesc;
	ZeroMemory(&boxViewDesc, sizeof(boxViewDesc));
	boxViewDesc.Format = boxTextureDesc.Format;
	boxViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	boxViewDesc.TextureCube.MipLevels = 1;
	boxViewDesc.TextureCube.MostDetailedMip = 0;

	//Create the Resource view
	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateShaderResourceView(boxTexture.Get(), &boxViewDesc, m_cubesTextureRes.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("SkyBox::CreateIndexBuffer(): CreateBuffer() failed", hr);
	}
}

void SkyBox::CreateConstMeshBuffer()
{
	MeshCB VsConstData;

	auto size = (UINT)std::ceil(sizeof(VsConstData) / 16.f) * 16;

	CD3D11_BUFFER_DESC cbDesc(
		size,
		D3D11_BIND_CONSTANT_BUFFER
	);

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&cbDesc, NULL, &m_constMeshBuffer)))
	{
		THROW_GAME_EXCEPTION("SkyBox::CreateConstMeshBuffer(): CreateBuffer() failed", hr);
	}
}

void SkyBox::CreateInputLayout()
{
	// setup IA layout
	HRESULT hr;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3DBlob* vertexShaderBLOB = g_D3D->shaderMgr->GetShaderBLOB(m_VSName);
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

void SkyBox::CreateVertexBuffer()
{
	m_vertexBuffer.Reset();

	CD3D11_BUFFER_DESC vDesc(
		(UINT)(sizeof(DirectX::XMFLOAT3) * m_vertices.size()),
		D3D11_BIND_VERTEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA vData;
	ZeroMemory(&vData, sizeof(D3D11_SUBRESOURCE_DATA));
	vData.pSysMem = m_vertices.data();
	vData.SysMemPitch = 0;
	vData.SysMemSlicePitch = 0;

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&vDesc, &vData, m_vertexBuffer.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("SkyBox::CreateVertexBuffer(): CreateBuffer() failed", hr);
	}
}

void SkyBox::CreateIndexBuffer()
{
	m_indexBuffer.Reset();

	CD3D11_BUFFER_DESC iDesc(
		(UINT)(sizeof(UINT) * m_indices.size()),
		D3D11_BIND_INDEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA iData;
	ZeroMemory(&iData, sizeof(D3D11_SUBRESOURCE_DATA));
	iData.pSysMem = m_indices.data();
	iData.SysMemPitch = 0;
	iData.SysMemSlicePitch = 0;

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&iDesc, &iData, m_indexBuffer.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("SkyBox::CreateIndexBuffer(): CreateBuffer() failed", hr);
	}
}