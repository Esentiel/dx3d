#pragma once

#include <string>
#include <memory>

namespace Assimp
{
	class Importer;
}


namespace Library
{
	class Mesh;

	class FileManager
	{
	public:
		FileManager();
		~FileManager();

		bool ReamModelFromFBX(const char * inFilePath, uint32_t id, Mesh* outMesh, uint32_t *outMeshNum);
	private:
		std::unique_ptr<Assimp::Importer> m_modelImporter;

	};

}