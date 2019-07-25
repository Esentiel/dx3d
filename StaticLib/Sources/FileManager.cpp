#include "stdafx.h"
#include "FileManager.h"

#include <vector>
#include <assert.h>
#include <DirectXMath.h>
#include <cstdlib>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

#include "Mesh.h"
#include "GameException.h"
#include "TextureManager.h"

using namespace Library;

struct MiniMesh
{
	aiMatrix4x4 transformations;
	unsigned int numMeshes;
	unsigned int* meshesIdx;
};

void TraverseMeshes(aiNode* rootNode, const aiMatrix4x4 &trans, std::vector<MiniMesh> &meshes)
{
	aiMatrix4x4 currTrans = rootNode->mTransformation * trans;
	if (rootNode->mNumMeshes)
	{
		MiniMesh mesh;
		mesh.meshesIdx = rootNode->mMeshes;
		mesh.numMeshes = rootNode->mNumMeshes;
		mesh.transformations = currTrans;

		meshes.push_back(mesh);
	}

	for (uint32_t i = 0; i < rootNode->mNumChildren; i++)
	{
		TraverseMeshes(rootNode->mChildren[i], currTrans, meshes);
	}
}

FileManager::FileManager() : 
	m_modelImporter(std::make_unique<Assimp::Importer>())
{
}


FileManager::~FileManager()
{
}

bool FileManager::ReadModelFromFBX(const char * inFilePath, uint32_t id, Mesh* outMesh, uint32_t *outMeshNum)
{
	// fetch data
	const aiScene* scene = m_modelImporter->ReadFile(inFilePath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_SortByPType);

	if (scene)
	{
		// nodes
		aiNode* rootNode = scene->mRootNode;

		std::vector<MiniMesh> tempMeshes;
		aiMatrix4x4 currTrans;

		TraverseMeshes(rootNode, currTrans, tempMeshes);
		for (const auto &miniM : tempMeshes)
		{
			if (id == *miniM.meshesIdx)
			{
				aiVector3D rot;
				aiVector3D scale;
				aiVector3D trans;

				miniM.transformations.Decompose(scale, rot, trans);

				DirectX::XMFLOAT3 DXrot(rot.x, rot.y, rot.z);
				DirectX::XMFLOAT3 DXscale(scale.x, scale.y, scale.z);
				DirectX::XMFLOAT3 DXtrans(trans.x, trans.y, trans.z);

				outMesh->Move(DXtrans);
				outMesh->Scale(DXscale);
				outMesh->Rotate(DXrot);
			}
		}

		// meshes
		uint32_t i = 0;
		uint32_t j = 0;
		*outMeshNum = scene->mNumMeshes;
		aiMesh* mesh = scene->mMeshes[id];
		uint32_t verticesNum = mesh->mNumVertices;
		uint32_t indicesNum = mesh->mNumFaces * 3;
		std::unique_ptr<DirectX::XMFLOAT3[]> vertices = std::make_unique<DirectX::XMFLOAT3[]>(verticesNum);
		std::unique_ptr<DirectX::XMFLOAT3[]> normals = std::make_unique<DirectX::XMFLOAT3[]>(verticesNum);
		std::unique_ptr<UINT[]> indices = std::make_unique<UINT[]>(indicesNum);
		
		for (; i < verticesNum; i++)
		{
			vertices[i].x = mesh->mVertices[i].x;
			vertices[i].y = mesh->mVertices[i].y;
			vertices[i].z = mesh->mVertices[i].z;
		}
		outMesh->SetVertices(std::move(vertices), verticesNum);

		for (i = 0; i < mesh->mNumFaces; i++)
		{
			indices[j++] = mesh->mFaces[i].mIndices[0];
			indices[j++] = mesh->mFaces[i].mIndices[1];
			indices[j++] = mesh->mFaces[i].mIndices[2];
		}
		outMesh->SetIndices(std::move(indices), indicesNum);

		if (mesh->HasNormals())
		{
			for (i = 0; i < verticesNum; i++)
			{
				normals[i].x = mesh->mNormals[i].x;
				normals[i].y = mesh->mNormals[i].y;
				normals[i].z = mesh->mNormals[i].z;
			}

			outMesh->SetNormals(std::move(normals));
		}

		// materials, textures
		if (scene->mNumMaterials)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			aiColor4D specularColor;
			aiColor4D diffuseColor;
			aiColor4D ambientColor;
			float shininess;

			aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &specularColor);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &diffuseColor);
			aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &ambientColor);
			aiGetMaterialFloat(material, AI_MATKEY_SHININESS, &shininess);
			
			for (unsigned int k = 0; k < material->GetTextureCount(aiTextureType_DIFFUSE); k++)
			{
				assert(k == 0);
				aiString texturePath;

				if (material->GetTexture(aiTextureType_DIFFUSE, k, &texturePath) ==	aiReturn_SUCCESS)
				{
					// Check if it's an embedded or external  texture.
					if (auto texture = scene->GetEmbeddedTexture(texturePath.C_Str()))
					{
						throw GameException("Engine doesn't work with embedded textures!");
					}
					else
					{
						g_D3D->textureMgr->LoadTexture(texturePath.C_Str());
						outMesh->SetTexturePath(texturePath.C_Str());
					}
				}
			}

			// texture coords
			if (mesh->mTextureCoords)
			{
				std::unique_ptr<DirectX::XMFLOAT2[]> textCoords_ = std::make_unique<DirectX::XMFLOAT2[]>(mesh->mNumVertices);

				for (uint32_t k = 0; k < mesh->mNumVertices; k++)
				{
					textCoords_[k].x = mesh->mTextureCoords[0][k].x;
					textCoords_[k].y = mesh->mTextureCoords[0][k].y;
				}

				outMesh->SetTextureCoords(std::move(textCoords_));
			}

			// todo: hard-code for a better look
			outMesh->Scale(DirectX::XMFLOAT3(.2f, 0.2f, 0.2f));
		}

		return true;
	}

	return false;
}

