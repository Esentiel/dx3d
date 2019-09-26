#include "stdafx.h"
#include "Mesh.h"

#include <d3d11.h>

#include "GameException.h"
#include "ShaderManager.h"
#include "TextureManager.h"
#include "Transformations.h"

using namespace Library;


void Mesh::CreateInputLayout()
{
	// setup IA layout
	HRESULT hr;
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENTS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3DBlob* vertexShaderBLOB = g_D3D->shaderMgr->GetShaderBLOB(m_vertexShaderName);
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

Mesh::Mesh() :
	m_transformations(std::make_unique<Transformations>()),
	m_dirtyVertex(true),
	m_flags(0)
{
	m_textures.resize(TextureType::Count);
}

Mesh::~Mesh()
{
}

ID3D11Buffer* Mesh::GetConstMeshBuffer() const
{
	return m_constMeshBuffer.Get();
}

ID3D11Buffer** Mesh::GetConstMeshBufferRef()
{
	return m_constMeshBuffer.GetAddressOf();
}

ID3D11Buffer** Mesh::GetVertexBufferRef()
{
	return m_vertexBuffer.GetAddressOf();
}

ID3D11Buffer** Mesh::GetVertexLightBufferRef()
{
	return m_vertexLightBuffer.GetAddressOf();
}

ID3D11Buffer* Mesh::GetIndexBuffer() const
{
	return m_indexBuffer.Get();
}

ID3D11InputLayout* Mesh::GetInputLayout() const
{
	return m_inputlayout.Get();
}

const DirectX::XMMATRIX* Mesh::GetModelTransform() const
{
	return m_transformations->GetModel();
}

const std::string& Mesh::GetVertexShader() const
{
	return m_vertexShaderName;
}

const std::string& Mesh::GetPixelShader() const
{
	return m_pixelShaderName;
}

const int Mesh::GetIndexCount() const
{
	return m_indexCnt;
}

void Mesh::SetVertices(std::unique_ptr<DirectX::XMFLOAT3[]> &&vertices, uint32_t cnt)
{
	m_vertices.swap(vertices);
	m_vertexCnt = cnt;

	m_vertexDataBuffer.resize(m_vertexCnt);
}

void Mesh::SetIndices(std::unique_ptr<UINT[]> &&indices, uint32_t cnt)
{
	m_indices.swap(indices);
	m_indexCnt = cnt;

	CreateIndexBuffer();
}

void Mesh::SetTextureCoords(std::unique_ptr<DirectX::XMFLOAT2[]> &&textureCoords)
{
	m_textCoords.swap(textureCoords);
}

void Mesh::SetNormals(std::unique_ptr<DirectX::XMFLOAT3[]> &&normals)
{
	m_normals.swap(normals);
}

void Mesh::Move(const DirectX::XMFLOAT3 &direction)
{
	m_transformations->Move(direction);
}

void Mesh::Rotate(const DirectX::XMFLOAT3 &rotation)
{
	m_transformations->Rotate(rotation);
}

void Mesh::Scale(const DirectX::XMFLOAT3 &scale)
{
	m_transformations->Scale(scale);
}

void Mesh::Initialize()
{
	assert(g_D3D->device);
	assert(g_D3D->shaderMgr);

	// todo: Mocking shaders names move into Material
	m_vertexShaderName = "VertexShader";
	m_pixelShaderName = "PixelShader";

	CreateInputLayout();
	CreateConstMeshBuffer();

	LoadVertexDataBuffer();
}

void Mesh::CreateConstMeshBuffer()
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
		THROW_GAME_EXCEPTION("CreateConstMeshBuffer(): CreateBuffer() failed", hr);
	}
}

void Mesh::CreateVertexBuffer()
{
	m_vertexBuffer.Reset();

	CD3D11_BUFFER_DESC vDesc(
		sizeof(Vertex) * m_vertexCnt,
		D3D11_BIND_VERTEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA vData;
	ZeroMemory(&vData, sizeof(D3D11_SUBRESOURCE_DATA));
	vData.pSysMem = m_vertexDataBuffer.data();
	vData.SysMemPitch = 0;
	vData.SysMemSlicePitch = 0;

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&vDesc, &vData, m_vertexBuffer.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("Mesh::CreateVertexBuffer(): CreateBuffer() failed", hr);
	}
}

void Mesh::CreateIndexBuffer()
{
	m_indexBuffer.Reset();

	CD3D11_BUFFER_DESC iDesc(
		sizeof(UINT) * m_indexCnt,
		D3D11_BIND_INDEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA iData;
	ZeroMemory(&iData, sizeof(D3D11_SUBRESOURCE_DATA));
	iData.pSysMem = m_indices.get();
	iData.SysMemPitch = 0;
	iData.SysMemSlicePitch = 0;

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&iDesc, &iData, m_indexBuffer.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("Mesh::CreateIndexBuffer(): CreateBuffer() failed", hr);
	}
}


void Mesh::LoadVertexDataBuffer()
{
	if (m_dirtyVertex)
	{
		for (int i = 0; i < m_vertexCnt; i++)
		{
			m_vertexDataBuffer[i].Position = m_vertices[i];
			m_vertexDataBuffer[i].TextCoord = m_textCoords[i];
			m_vertexDataBuffer[i].Normal = m_normals[i];

			if (m_tangents)
			{
				m_vertexDataBuffer[i].Bitangents = m_bitangents[i];
				m_vertexDataBuffer[i].Tangents = m_tangents[i];
			}
		}

		CreateVertexBuffer();
		CreateVertexLightBuffer();

		m_dirtyVertex = false;
	}
}

const std::string& Library::Mesh::GetTexturePath(TextureType type) const
{
	return m_textures[type];
}

void Library::Mesh::SetTexturePath(const std::string& path, TextureType type)
{
	m_textures[type] = path;
}

void Library::Mesh::SetTangents(std::unique_ptr<DirectX::XMFLOAT3[]> &&tangents, std::unique_ptr<DirectX::XMFLOAT3[]> &&bitangents)
{
	m_tangents.swap(tangents);
	m_bitangents.swap(bitangents);
}

void Library::Mesh::SetFlag(unsigned int flag)
{
	m_flags |= flag;
}

void Library::Mesh::UnsetFlag(unsigned int flag)
{
	m_flags &= flag;
}

bool Library::Mesh::GetFlag(unsigned int flag) const
{
	return m_flags & flag;
}

void Library::Mesh::CreateVertexLightBuffer()
{
	m_vertexLightBuffer.Reset();

	CD3D11_BUFFER_DESC vDesc(
		sizeof(DirectX::XMFLOAT3) * m_vertexCnt,
		D3D11_BIND_VERTEX_BUFFER
	);

	D3D11_SUBRESOURCE_DATA vData;
	ZeroMemory(&vData, sizeof(D3D11_SUBRESOURCE_DATA));
	vData.pSysMem = m_vertices.get();
	vData.SysMemPitch = 0;
	vData.SysMemSlicePitch = 0;

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&vDesc, &vData, m_vertexLightBuffer.GetAddressOf())))
	{
		THROW_GAME_EXCEPTION("Mesh::CreateVertexLightBuffer(): CreateBuffer() failed", hr);
	}
}
