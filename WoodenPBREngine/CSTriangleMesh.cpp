#include "pch.h"
#include "CSTriangleMesh.h"
#include "CSTriangle.h"
#include "MEngine.h"

WPBR_BEGIN

DECL_OUT_COMP_DATA(CTriangleMesh)


uint32_t STriangleMesh::create(
						 CTransform world,
						 CTriangleMesh mesh)
{
	MEngine& engine = MEngine::getInstance();
	uint32_t hEntity = engine.createEntity();

	engine.addComponent<CTransform>(hEntity, std::move(world));

	// Convert elements to world space
	for (uint32_t i = 0; i < mesh.nVertices; i++)
	{
		mesh.positions[i] = world(mesh.positions[i]);

		if (!mesh.normals.empty())
		{
			mesh.normals[i] = world(mesh.normals[i]);
		}

		if (!mesh.tangents.empty())
		{
			mesh.tangents[i] = world(mesh.tangents[i]);
		}
	}

 	engine.addComponent<CTriangleMesh>(hEntity, std::move(mesh));

	return hEntity;
}

void STriangleMesh::generateTriangles(WECS& engine,
									  HEntity hEntity,
									  const CTriangleMesh& triangleMesh,
									   CMaterialHandle hMaterial)
{
	for (uint32_t i = 0; i < triangleMesh.nTriangles; i++)
	{
		
		HEntity h = engine.createEntity();
		CTriangle triangle;
		triangle.index = i;
		triangle.hMesh = hEntity;
		engine.addComponent<CTriangle>(h, std::move(triangle));
		engine.addComponent<CBounds>(h);
		engine.addComponent<CCentroid>(h);
		engine.addComponent<CMapUVRequests>(h);
		engine.addComponent<CMaterialHandle>(h, hMaterial);

	}
}


void JobTriangleMeshGenerateTriangles::update(WECS* ecs, HEntity hEntity, CTriangleMesh& mesh, CMaterialHandle& hMaterial)
{
	STriangleMesh::generateTriangles(*ecs, hEntity, mesh, hMaterial);
}

WPBR_END

