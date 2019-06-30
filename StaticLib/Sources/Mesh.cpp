#include "stdafx.h"
#include "Mesh.h"

#include <d3d11.h>

#include "GameException.h"
#include "ShaderManager.h"
#include "TextureManager.h"

using namespace Library;



Mesh::Mesh(std::unique_ptr<Library::Vertex[]> vertices, int vertexCnt, std::unique_ptr<UINT[]> indices, int indexCnt, std::string diffuseTexture) :
	m_vertexdata(std::move(vertices)),
	m_vertexCnt(vertexCnt),
	m_indexData(std::move(indices)),
	m_indexCnt(indexCnt),
	m_diffuseTexture(diffuseTexture)
{
	Initialize();
}


Mesh::~Mesh()
{
}

void Mesh::CreateInputLayout()
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
			throw GameException("CreateInputLayout() failed", hr);
		}
	}
	else
	{
		throw GameException("Mesh::CreateInputLayout() failed as vertexShaderBLOB is null");
	}
}

ID3D11Buffer* Library::Mesh::GetConstBuffer() const
{
	return m_constBuffer.Get();
}

ID3D11Buffer** Library::Mesh::GetConstBufferRef()
{
	return m_constBuffer.GetAddressOf();
}

ID3D11Buffer** Library::Mesh::GetVertexBufferRef()
{
	return m_vertexBuffer.GetAddressOf();
}

ID3D11Buffer* Library::Mesh::GetIndexBuffer() const
{
	return m_indexBuffer.Get();
}

ID3D11InputLayout* Library::Mesh::GetInputLayout() const
{
	return m_inputlayout.Get();
}

ID3D11SamplerState** Library::Mesh::GetSampler()
{
	return m_sampler.GetAddressOf();
}

const DirectX::XMMATRIX* Library::Mesh::GetModelTransform() const
{
	return m_modelTransform.get();
}

const Library::Vertex* Library::Mesh::GetVertexData() const
{
	return m_vertexdata.get();
}

const UINT* Library::Mesh::GetIndexData() const
{
	return m_indexData.get();
}

const std::string& Library::Mesh::GetVertexShader() const
{
	return m_vertexShaderName;
}

const std::string& Library::Mesh::GetPixelShader() const
{
	return m_pixelShaderName;
}

const int Library::Mesh::GetIndexCount() const
{
	return m_indexCnt;
}

const std::string& Library::Mesh::GetDiffuseTexture() const
{
	return m_diffuseTexture;
}

void Library::Mesh::Initialize()
{
	assert(g_D3D->device);
	assert(g_D3D->shaderMgr);

	// todo: Mocking shaders names
	m_vertexShaderName = "VertexShader";
	m_pixelShaderName = "PixelShader";

	// todo: Mocking model transform
	float angle = 90.0f;
	const DirectX::XMVECTOR rotationAxis = DirectX::XMVectorSet(0, 1, 1, 0);
	m_modelTransform.reset(new DirectX::XMMATRIX(DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle))));

	CreateInputLayout();
	CreateConstantBuffer();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateSampler();
	g_D3D->textureMgr->LoadTexture(m_diffuseTexture);
}

void Library::Mesh::CreateConstantBuffer()
{
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

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&cbDesc, NULL, &m_constBuffer)))
	{
		throw GameException("CreateBuffer() failed", hr);
	}
}

void Library::Mesh::CreateVertexBuffer()
{
	CD3D11_BUFFER_DESC vDesc(
		sizeof(Vertex) * m_vertexCnt,
		D3D11_BIND_VERTEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA vData;
	ZeroMemory(&vData, sizeof(D3D11_SUBRESOURCE_DATA));
	vData.pSysMem = m_vertexdata.get();
	vData.SysMemPitch = 0;
	vData.SysMemSlicePitch = 0;

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&vDesc, &vData, m_vertexBuffer.GetAddressOf())))
	{
		throw GameException("Mesh::CreateVertexBuffer(): CreateBuffer() failed", hr);
	}
}

void Library::Mesh::CreateIndexBuffer()
{
	CD3D11_BUFFER_DESC iDesc(
		sizeof(UINT) * m_indexCnt,
		D3D11_BIND_INDEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA iData;
	ZeroMemory(&iData, sizeof(D3D11_SUBRESOURCE_DATA));
	iData.pSysMem = m_indexData.get();
	iData.SysMemPitch = 0;
	iData.SysMemSlicePitch = 0;

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&iDesc, &iData, m_indexBuffer.GetAddressOf())))
	{
		throw GameException("Mesh::CreateIndexBuffer(): CreateBuffer() failed", hr);
	}
}

void Library::Mesh::CreateSampler()
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
	if (FAILED(hr = g_D3D->device->CreateSamplerState(&samplerDesc, m_sampler.GetAddressOf())))
	{
		throw GameException("CreateSamplerState() failed", hr);
	}
}
