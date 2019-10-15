#pragma once

#include "pch.h"
#include "CSTriangleMesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

WPBR_BEGIN

namespace MeshTools
{

	CTriangleMesh loadMesh(const std::string& filename)
	{
		Assimp::Importer Importer;

		const auto* Scene = Importer.ReadFile(filename,
											  aiProcess_Triangulate |
											  aiProcess_ConvertToLeftHanded);

		if (Scene == nullptr)
		{
			throw std::invalid_argument("The file " + filename + " can't be opened");
		}

		auto* RootNode = (aiMesh*)(Scene->mMeshes[0]);

		CTriangleMesh mesh;
		mesh.positions.resize(RootNode->mNumVertices);

		bool hasNormals = RootNode->mNormals != nullptr;
		bool hasTangents = RootNode->mTangents != nullptr;
		bool hasUVs = RootNode->mTextureCoords[0] != nullptr;

		if (hasNormals)
		{
			mesh.normals.resize(RootNode->mNumVertices);
		}

		if (hasTangents)
		{
			mesh.tangents.resize(RootNode->mNumVertices);
		}

		if (hasUVs)
		{
			mesh.uvs.resize(RootNode->mNumVertices);
		}

		for (auto iVertex = 0; iVertex < RootNode->mNumVertices; ++iVertex)
		{
			mesh.positions[iVertex] = {
				RootNode->mVertices[iVertex].x,
				RootNode->mVertices[iVertex].y,
				RootNode->mVertices[iVertex].z
			};

			if (hasNormals)
			{
				mesh.normals[iVertex] = {
					RootNode->mNormals[iVertex].x,
					RootNode->mNormals[iVertex].y,
					RootNode->mNormals[iVertex].z,
				};
			}

			if (hasTangents)
			{
				mesh.tangents[iVertex] = {
					RootNode->mTangents[iVertex].x,
					RootNode->mTangents[iVertex].y,
					RootNode->mTangents[iVertex].z,
				};
			}

			if (hasUVs)
			{
				mesh.uvs[iVertex] = {
					RootNode->mTextureCoords[0][iVertex].x,
					RootNode->mTextureCoords[0][iVertex].y,
				};
			}
		}

		for (auto iFace = 0; iFace < RootNode->mNumFaces; ++iFace)
		{
			auto Face = RootNode->mFaces[iFace];
			for (auto iIndex = 0; iIndex < Face.mNumIndices; ++iIndex)
			{
				mesh.iVertices.push_back(Face.mIndices[iIndex]);
			}
		}

		mesh.nTriangles = mesh.iVertices.size()/3;
		mesh.nVertices = mesh.positions.size();

		return mesh;
	}

	CTriangleMesh createQuad(
		float Width, float Height) 
	{
		CTriangleMesh mesh;

		mesh.nTriangles = 2;
		mesh.nVertices = 4;

		const auto WidthHalf = Width / 2.0f;
		const auto DepthHalf = Height / 2.0f;


		/*{
			-WidthHalf, 0.0f, DepthHalf, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f
		},
			{ WidthHalf, 0.0f, DepthHalf, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f },
			{ -WidthHalf, 0.0f, -DepthHalf, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f },
			{ WidthHalf, 0.0f, -DepthHalf, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f },*/

		// n, t, uv
		mesh.positions = {
			{ -WidthHalf, 0.0f, DepthHalf},
			{ WidthHalf, 0.0f, DepthHalf},
			{ -WidthHalf, 0.0f, -DepthHalf},
			{ WidthHalf, 0.0f, -DepthHalf}
		};

		mesh.normals = {
			{ 0.0f, 1.0f, 0.0f},
			{ 0.0f, 1.0f, 0.0f},
			{0.0f, 1.0f, 0.0f},
			{ 0.0f, 1.0f, 0.0f}
		};

		mesh.tangents = {
			{ 1.0f, 0.0f, 0.0f},
			{ 1.0f, 0.0f, 0.0f},
			{1.0f, 0.0f, 0.0f},
			{ 1.0f, 0.0f, 0.0f}
		};

		mesh.uvs = {
			{ 0.0f, 1.0f},
			{1.0f, 1.0f},
			{0.0f, 0.0f },
			{1.0f, 0.0f}
		};

		mesh.iVertices = {
			0, 1, 2, 1, 2, 3
		};

		return mesh;
	}

}


WPBR_END

