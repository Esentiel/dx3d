#include "stdafx.h"
#include "RenderScene.h"
#include "Mesh.h"
#include "GameException.h"
#include "LightSource.h"

#include <d3d11.h>
#include <DirectXMath.h>

using namespace Library;

RenderScene::RenderScene()
{
	CreateConstSceneBuffer();
}


RenderScene::~RenderScene()
{
}

std::vector<std::unique_ptr<Library::Mesh>>::const_iterator Library::RenderScene::BeginMesh() const
{
	return m_meshes.cbegin();
}

std::vector<std::unique_ptr<Library::Mesh>>::const_iterator Library::RenderScene::EndMesh() const
{
	return m_meshes.cend();
}

void Library::RenderScene::AddMesh(std::unique_ptr<Mesh> mesh)
{
	m_meshes.push_back(std::move(mesh));
}

ID3D11Buffer* Library::RenderScene::GetConstSceneBuffer() const
{
	return m_constSceneBuffer.Get();
}

ID3D11Buffer** Library::RenderScene::GetConstSceneBufferRef()
{
	return m_constSceneBuffer.GetAddressOf();
}

void Library::RenderScene::CreateConstSceneBuffer()
{
	

	SceneCB sceneCB;

	CD3D11_BUFFER_DESC cbDesc(
		64, // todo: hardcoded size. should be division of 16 for CB
		D3D11_BIND_CONSTANT_BUFFER
	);

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&cbDesc, NULL, &m_constSceneBuffer)))
	{
		throw GameException("CreateConstSceneBuffer(): CreateBuffer() failed", hr);
	}
}
