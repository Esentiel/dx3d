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
		DirectX::XMFLOAT4X4 ViewProj;
		DirectX::XMFLOAT4X4 ShadowMapMatrix;
		DirectX::XMFLOAT4 Emissive;
		DirectX::XMFLOAT4 Ambient;
		DirectX::XMFLOAT4 Diffuse;
		DirectX::XMFLOAT4 Specular;
		float specularPower;
		int CalcLight;
		int HasNormalMap;
		int HasSpecularMap;
	};

	struct MeshLightCB
	{
		DirectX::XMFLOAT4X4 WorldViewLightProj;
	};

	struct Vertex
	{
		DirectX::XMFLOAT3 Position;
		DirectX::XMFLOAT2 TextCoord;
		DirectX::XMFLOAT3 Normal;
		DirectX::XMFLOAT3 Tangents;
		DirectX::XMFLOAT3 Bitangents;

		Vertex() {}
		Vertex(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT2 textCoord) : Position(pos), TextCoord(textCoord) {}
	};

	class Transformations;

	class Mesh
	{
	public:
		enum TextureType 
		{
			DiffuseTexture = 0,
			NormalTexture,
			SpecularTexture,
			Count
		};
		enum MeshFlags
		{
			CalcLight = 0x1,
			UseNormalMap = 0x2,
			UseSpecularMap = 0x4,

			LastFlag = UseSpecularMap
		};
	public:
		Mesh();
		~Mesh();
		void Initialize();

		ID3D11Buffer* GetConstMeshBuffer() const;
		ID3D11Buffer** GetConstMeshBufferRef();
		ID3D11Buffer** GetVertexBufferRef();
		ID3D11Buffer* GetIndexBuffer() const;

		ID3D11Buffer** GetVertexLightBufferRef();
		
		void Move(const DirectX::XMFLOAT3 &direction);
		void Rotate(const DirectX::XMFLOAT3 &rotation);
		void Scale(const DirectX::XMFLOAT3 &scale);
		const DirectX::XMMATRIX* GetModelTransform() const;

		const int GetIndexCount() const;
		const std::string& GetTexturePath(TextureType type) const;

		void SetVertices(std::unique_ptr<DirectX::XMFLOAT3[]> &&vertices, uint32_t cnt);
		void SetIndices(std::unique_ptr<UINT[]> &&indices, uint32_t cnt);
		void SetTexturePath(const std::string& path, TextureType type);
		void SetTextureCoords(std::unique_ptr<DirectX::XMFLOAT2[]> &&textureCoords);
		void SetNormals(std::unique_ptr<DirectX::XMFLOAT3[]> &&normals);
		void SetTangents(std::unique_ptr<DirectX::XMFLOAT3[]> &&tangents, std::unique_ptr<DirectX::XMFLOAT3[]> &&bitangents);

		void SetFlag(unsigned int flag);
		void UnsetFlag(unsigned int flag);
		bool GetFlag(unsigned int flag) const;

		ID3D11InputLayout* GetInputLayout() const; // todo: should move layout out from Mesh, as long as I will create several types of mashes with different layout (put layout into Material cause it is related with vertex shader input...)
		const std::string& GetVertexShader() const; // todo: move to Material
		const std::string& GetPixelShader() const; // todo: move to Material
	private:
		void LoadVertexDataBuffer();
		void CreateInputLayout();
		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateConstMeshBuffer();

		void CreateVertexLightBuffer(); // ass light input layout to support SM and SB

		std::unique_ptr<Transformations> m_transformations;

		std::unique_ptr<DirectX::XMFLOAT3[]> m_vertices;
		std::unique_ptr<UINT[]> m_indices;
		std::unique_ptr<DirectX::XMFLOAT3[]> m_normals;
		std::unique_ptr<DirectX::XMFLOAT3[]> m_tangents;
		std::unique_ptr<DirectX::XMFLOAT3[]> m_bitangents;
		std::unique_ptr<DirectX::XMFLOAT2[]> m_textCoords;

		std::vector<Vertex> m_vertexDataBuffer;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constMeshBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexLightBuffer;
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputlayout;

		std::string m_vertexShaderName;
		std::string m_pixelShaderName;
		std::vector<std::string> m_textures;
		int m_vertexCnt;
		int m_indexCnt;

		bool m_dirtyVertex;
		unsigned int m_flags;
	};
}