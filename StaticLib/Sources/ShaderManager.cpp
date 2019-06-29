#include "stdafx.h"
#include "ShaderManager.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#include "GameException.h"

static const std::string s_shaderDir = "../Bin/x64/Debug/";

using namespace Library;

ShaderManager::ShaderManager()
{
}


ShaderManager::~ShaderManager()
{
}

void ShaderManager::Initialize()
{
	// todo: Mocking logic here. 
	// supposed to scan dir and find all shaders and load them. Also distinct between shader types.
	// but I'm to lazy now xD

	// load shaders
	// vertex
	assert(g_D3D->device);

	HRESULT hr;
	Microsoft::WRL::ComPtr<ID3DBlob> VS_Buffer, PS_Buffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

	if (FAILED(hr = D3DReadFileToBlob(L"../Bin/x64/Debug/VertexShader.cso", &VS_Buffer)))
	{
		throw GameException("D3DReadFileToBlob() for VS failed", hr);
	}
	if (FAILED(hr = g_D3D->device->CreateVertexShader(VS_Buffer->GetBufferPointer(), VS_Buffer->GetBufferSize(), nullptr, vertexShader.GetAddressOf())))
	{
		throw GameException("CreateVertexShader() failed", hr);
	}
	m_shaders.insert({"VertexShader", std::move(VS_Buffer)});
	m_vertexShaders.insert({"VertexShader", std::move(vertexShader)});

	// pixel
	if (FAILED(hr = D3DReadFileToBlob(L"../Bin/x64/Debug/PixelShader.cso", &PS_Buffer)))
	{
		throw GameException("D3DReadFileToBlob() for PS failed", hr);
	}
	if (FAILED(hr = g_D3D->device->CreatePixelShader(PS_Buffer->GetBufferPointer(), PS_Buffer->GetBufferSize(), nullptr, pixelShader.GetAddressOf())))
	{
		throw GameException("CreateVertexShader() failed", hr);
	}
	m_shaders.insert({ "PixelShader", std::move(PS_Buffer) });
	m_pixelShaders.insert({"PixelShader", std::move(pixelShader)});


}

ID3DBlob* Library::ShaderManager::GetShaderBLOB(const std::string &name) const
{
	auto it = m_shaders.find(name);
	if (it != m_shaders.end())
		return it->second.Get();
	else
		return NULL;
}

ID3D11VertexShader* Library::ShaderManager::GetVertexShader(const std::string &name) const
{
	auto it = m_vertexShaders.find(name);
	if (it != m_vertexShaders.end())
		return it->second.Get();
	else
		return NULL;
}

ID3D11PixelShader* Library::ShaderManager::GetPixelShader(const std::string &name) const
{
	auto it = m_pixelShaders.find(name);
	if (it != m_pixelShaders.end())
		return it->second.Get();
	else
		return NULL;
}
