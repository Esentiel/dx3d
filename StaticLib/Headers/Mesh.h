#pragma once

#include <string>
#include <memory>
#include <wrl.h>
#include <DirectXMath.h>

struct ID3D11Buffer;
struct ID3D11InputLayout;
struct ID3D11Device;

namespace Library
{
	struct Vertex  // todo: 2 places to edit vertex(
	{
		DirectX::XMFLOAT3 Position;

		Vertex() {}
		Vertex(DirectX::XMFLOAT3 pos) : Position(pos) {}
	};

	class Mesh
	{
	public:
		Mesh(std::unique_ptr<Vertex[]> vertices, int vertexCnt, std::unique_ptr<UINT[]> indices, int indexCnt);
		~Mesh();

		ID3D11Buffer* GetConstBuffer() const;
		ID3D11Buffer** GetConstBufferRef();
		ID3D11Buffer** GetVertexBufferRef();
		ID3D11Buffer* GetIndexBuffer() const;
		ID3D11InputLayout* GetInputLayout() const;
		const DirectX::XMMATRIX* GetModelTransform() const;
		const Vertex* GetVertexData() const;
		const UINT* GetIndexData() const;

		const std::string& GetVertexShader() const;
		const std::string& GetPixelShader() const;
		const int GetIndexCount() const;

		void Initialize();
	private:
		void CreateInputLayout();
		void CreateConstantBuffer();
		void CreateVertexBuffer();
		void CreateIndexBuffer();

		std::unique_ptr<DirectX::XMMATRIX> m_modelTransform;
		std::unique_ptr<Vertex[]> m_vertexdata;
		std::unique_ptr<UINT[]> m_indexData;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_Inputlayout;

		std::string m_vertexShaderName;
		std::string m_pixelShaderName;
		int m_vertexCnt;
		int m_indexCnt;
	};
}