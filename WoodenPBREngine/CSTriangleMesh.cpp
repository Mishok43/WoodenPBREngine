#include "pch.h"
#include "CSTriangleMesh.h"
#include "CSTriangle.h"
#include "MEngine.h"

WPBR_BEGIN


DECL_OUT_COMP_DATA(CTriangleMeshHandle)
DECL_OUT_COMP_DATA(CTriangleMesh)


uint32_t STriangleMesh::create(
						 CTransform world,
						 CTriangleMesh mesh)
{
	MEngine& engine = MEngine::getInstance();
	uint32_t hEntity = engine.createEntity();

	engine.addComponent<CTransform>(hEntity, std::move(world));
	engine.addComponent<CTriangleMesh>(hEntity, std::move(mesh));

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

	return hEntity;
}


void STriangleMesh::generateTriangles(MEngine& engine,
							HEntity hEntity,
						   const CTriangleMesh& triangleMesh)
{
	for (uint32_t i = 0; i < triangleMesh.nTriangles; i++)
	{
		
		HEntity hEntity = engine.createEntity();
		engine.addComponent<CTriangle>(hEntity, CTriangle{ i });
		engine.addComponent<CTriangleMeshHandle>(hEntity, CTriangleMeshHandle(hEntity));
	}
}

WPBR_END

