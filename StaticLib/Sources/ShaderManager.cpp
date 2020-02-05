#include "stdafx.h"
#include "ShaderManager.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#include <filesystem>

#include "GameException.h"

using namespace Library;

ShaderManager::ShaderManager()
{
	std::filesystem::path p = std::filesystem::path(g_D3D->executablePath);
	m_shaderFolderPath = p.parent_path().wstring() + L"\\Shaders\\";
}


ShaderManager::~ShaderManager()
{
}

void ShaderManager::Initialize()
{
	assert(g_D3D->device);

	for (const auto & entry : std::filesystem::directory_iterator(m_shaderFolderPath))
	{
		HRESULT hr;
		Microsoft::WRL::ComPtr<ID3DBlob> buffer;

		std::wstring shaderPath = entry.path().wstring();
		std::string shadername = entry.path().stem().string();

		if (FAILED(hr = D3DReadFileToBlob(shaderPath.c_str(), buffer.GetAddressOf())))
		{
			THROW_GAME_EXCEPTION(std::string("D3DReadFileToBlob() failed for: ") + entry.path().string(), hr);
		}

		if (shadername._Starts_with("Vertex"))
		{
			Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
			if (FAILED(hr = g_D3D->device->CreateVertexShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), nullptr, vertexShader.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION(std::string("CreateVertexShader() failed for: ")  + entry.path().string(), hr);
			}
			m_vertexShaders.insert({ shadername, std::move(vertexShader) });
		}
		else if (shadername._Starts_with("Pixel"))
		{
			Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
			if (FAILED(hr = g_D3D->device->CreatePixelShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), nullptr, pixelShader.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION(std::string("CreatePixelShader() failed for: ")  + entry.path().string(), hr);
			}
			m_pixelShaders.insert({ shadername, std::move(pixelShader) });
		}
		else if (shadername._Starts_with("Compute"))
		{
			Microsoft::WRL::ComPtr<ID3D11ComputeShader> computeShader;
			if (FAILED(hr = g_D3D->device->CreateComputeShader(buffer->GetBufferPointer(), buffer->GetBufferSize(), nullptr, computeShader.GetAddressOf())))
			{
				THROW_GAME_EXCEPTION(std::string("CreateComputeShader() failed for: ") + entry.path().string(), hr);
			}
			m_computeShaders.insert({ shadername, std::move(computeShader) });
		}
		
		m_shaders.insert({ shadername, std::move(buffer) });
	}
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

ID3D11ComputeShader* Library::ShaderManager::GetComputeShader(const std::string &name) const
{
	auto it = m_computeShaders.find(name);
	if (it != m_computeShaders.end())
		return it->second.Get();
	else
		return NULL;
}
