#include "stdafx.h"
#include "RenderScene.h"
#include "Mesh.h"
#include "GameException.h"
#include "LightSource.h"
#include "SkyBox.h"

#include <d3d11.h>
#include <DirectXMath.h>

using namespace Library;

RenderScene::RenderScene()
{
	CreateConstSceneBuffer();
	m_skyBox.reset(new SkyBox);
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

void Library::RenderScene::DrawSkyBox()
{
	m_skyBox->Draw(this);
}

void Library::RenderScene::CreateConstSceneBuffer()
{
	int sizeCb = (int)std::ceil(sizeof(SceneCB) / 16.f) * 16;

	CD3D11_BUFFER_DESC cbDesc(
		sizeCb,
		D3D11_BIND_CONSTANT_BUFFER
	);

	HRESULT hr;
	if (FAILED(hr = g_D3D->device->CreateBuffer(&cbDesc, NULL, &m_constSceneBuffer)))
	{
		THROW_GAME_EXCEPTION("CreateConstSceneBuffer(): CreateBuffer() failed", hr);
	}
}
