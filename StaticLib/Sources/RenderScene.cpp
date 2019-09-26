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
	m_lights.reset(new LightSource[MAX_LIGHT_SOURCES]);
	
	// todo: hard-coded lights to test
	m_lights.get()[0].Type = 3;
	m_lights.get()[0].LightPos = DirectX::XMFLOAT4(-20.5f, 46.8f, 46.8f, 1.0f);
	m_lights.get()[0].LightDir = DirectX::XMFLOAT4(0.41f, -0.39f, -0.83f, 0.0f);
	m_lights.get()[0].LightPower = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_lights.get()[0].ConstantAttenuation = 1.0f;
	m_lights.get()[0].LinearAttenuation = 0.01f;
	m_lights.get()[0].QuadraticAttenuation = 0.0f;
	m_lights.get()[0].SpotAngle = DirectX::XMConvertToRadians(30.0f);

	m_lights.get()[1].Type = 1;
	m_lights.get()[1].LightPos = DirectX::XMFLOAT4(-20.5f, 36.8f, 16.8f, 1.0f);
	m_lights.get()[1].LightDir = DirectX::XMFLOAT4(-0.41f, -0.69f, -0.43f, 0.0f);
	m_lights.get()[1].LightPower = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_lights.get()[1].ConstantAttenuation = 1.0f;
	m_lights.get()[1].LinearAttenuation = 0.01f;
	m_lights.get()[1].QuadraticAttenuation = 0.0f;
	m_lights.get()[1].SpotAngle = DirectX::XMConvertToRadians(30.0f);
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

const Library::LightSource* Library::RenderScene::GetSceneLights() const
{
	return m_lights.get();
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
