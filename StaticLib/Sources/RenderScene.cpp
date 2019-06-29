#include "stdafx.h"
#include "RenderScene.h"
#include "Mesh.h"

using namespace Library;

RenderScene::RenderScene()
{
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
