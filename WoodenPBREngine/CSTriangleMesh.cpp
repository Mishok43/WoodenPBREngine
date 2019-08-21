#include "CSTriangleMesh.h"
#include "MEngine.h"

WPBR_BEGIN
uint32_t STriangleMesh::create(
						 CTransform world,
						 CTriangleMesh mesh,
						 CPositionArray positions,
						 CNormalArray* normals = nullptr,
						 CTangentArray* tangents = nullptr,
						 CUVArray* uvs = nullptr)
{
	MEngine& engine = MEngine::getInstance();
	uint32_t hEntity = engine.createEntity();

	engine.addComponent<CTransform>(hEntity, std::move(world));
	engine.addComponent<CTriangleMesh>(hEntity, std::move(mesh));
	engine.addComponent<CPositionArray>(hEntity, std::move(positions));
	if (normals)
	{
		engine.addComponent<CNormalArray>(hEntity, std::move(*normals));
	}
	if (tangents)
	{
		engine.addComponent<CTangentArray>(hEntity, std::move(*normals));
	}
	if (uvs)
	{
		engine.addComponent<CUVArray>(hEntity, std::move(*normals));
	}


	// Convert elements to world space
	for (uint32_t i = 0; i < mesh.nVertices; i++)
	{
		positions.data[i] = world(positions.data[i]);

		if (normals)
		{
			normals->data[i] = world(normals->data[i]);
		}

		if (tangents)
		{
			tangents->data[i] = world(tangents->data[i]);
		}
	}

	return hEntity;
}


WPBR_END

