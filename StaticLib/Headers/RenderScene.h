#pragma once

#include <memory>
#include <vector>
#include <wrl.h>

#include "LightSource.h"

struct ID3D11Buffer;

namespace Library
{
	class Mesh;

	struct SceneCB
	{
		DirectX::XMFLOAT3 EyePos;
		LightSource Light;
	};

	class RenderScene
	{
	public:
		RenderScene();
		~RenderScene();

		std::vector<std::unique_ptr<Mesh>>::const_iterator BeginMesh() const;
		std::vector<std::unique_ptr<Mesh>>::const_iterator EndMesh() const;

		void AddMesh(std::unique_ptr<Mesh> mesh);

		ID3D11Buffer* GetConstSceneBuffer() const;
		ID3D11Buffer** GetConstSceneBufferRef();

	private:
		void CreateConstSceneBuffer();
		
		std::vector<std::unique_ptr<Mesh>> m_meshes;
		
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_constSceneBuffer;
	};
}
