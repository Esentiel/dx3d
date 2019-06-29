#pragma once

#include <memory>
#include <vector>

namespace Library
{
	class Mesh;
	class RenderScene
	{
	public:
		RenderScene();
		~RenderScene();

		std::vector<std::unique_ptr<Mesh>>::const_iterator BeginMesh() const;
		std::vector<std::unique_ptr<Mesh>>::const_iterator EndMesh() const;

		void AddMesh(std::unique_ptr<Mesh> mesh);

	private:
		std::vector<std::unique_ptr<Mesh>> m_meshes;
	};
}
