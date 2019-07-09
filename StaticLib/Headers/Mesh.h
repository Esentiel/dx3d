#pragma once

#include <string>
#include <memory>
#include <wrl.h>
#include <DirectXMath.h>
#include <vector>

struct ID3D11Buffer;
struct ID3D11InputLayout;
struct ID3D11Device;

namespace Library
{
	struct MeshCB
	{
		DirectX::XMFLOAT4X4 WorldViewProj;
		DirectX::XMFLOAT4X4 World;
		float DiffuseIntensity;
		float EmissiveK;
		float AmbientK;
		float Roughness;
	};

	struct Vertex  // todo: 2 places to edit vertex(
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 TextCoord;
		DirectX::XMFLOAT3 Normal;

		Vertex() {}
		Vertex(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT2 textCoord) : Position(pos), TextCoord(textCoord) {}
	};

	class Transformations;

	class Mesh
	{
	public:
		Mesh();
		~Mesh();

		ID3D11Buffer* GetConstMeshBuffer() const;
		ID3D11Buffer** GetConstMeshBufferRef();
		ID3D11Buffer** GetVertexBufferRef();
		ID3D11Buffer* GetIndexBuffer() const;
		ID3D11InputLayout* GetInputLayout() const; // todo: should move layout out from Mesh, as long as I will create several types of mashes with differemt layout
		const DirectX::XMMATRIX* GetModelTransform() const;
		const DirectX::XMFLOAT3* GetVertexData() const;
		const UINT* GetIndexData() const;

		const std::string& GetVertexShader() const;
		const std::string& GetPixelShader() const;
		const int GetIndexCount() const;
		const std::string& GetDiffuseTexture() const;

		//
		void LoadVertexDataBuffer();

		void SetVertices(std::unique_ptr<DirectX::XMFLOAT3[]> &&vertices, uint32_t cnt);
		void SetIndices(std::unique_ptr<UINT[]> &&indices, uint32_t cnt);
		void SetTexturePath(const std::string& path);
		void SetTextureCoords(std::unique_ptr<DirectX::XMFLOAT2[]> &&textureCoords);
		void SetNormals(std::unique_ptr<DirectX::XMFLOAT3[]> &&normals);

		void Move(const DirectX::XMFLOAT3 &direction);
		void Rotate(const DirectX::XMFLOAT3 &rotation);
		void Scale(const DirectX::XMFLOAT3 &scale);
		
		//

		void Initialize();
	private:
		void CreateInputLayout();
		void CreateConstMeshBuffer();
		void CreateVertexBuffer();
		void CreateIndexBuffer();

		std::unique_ptr<Transformations> m_transformations;

		std::unique_ptr<DirectX::XMFLOAT3[]> m_vertices;
		std::unique_ptr<UINT[]> m_indices;
		std::unique_ptr<DirectX::XMFLOAT3[]> m_normals;
		std::unique_ptr<DirectX::XMFLOAT2[]> m_textCoords;

		std::vector<Vertex> m_vertexDataBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constMeshBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputlayout;

		std::string m_diffuseTexture;

		std::string m_vertexShaderName;
		std::string m_pixelShaderName;
		int m_vertexCnt;
		int m_indexCnt;

		bool m_dirtyVertex;
	};
}