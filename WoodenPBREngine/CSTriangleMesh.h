#pragma once

#include "pch.h"
#include "CTransform.h"
#include "MEngine.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "CMaterial.h"
WPBR_BEGIN


struct CTriangleMesh
{
	CTriangleMesh()
	{
	};

	CTriangleMesh(uint32_t nTriangle, uint32_t nVertex,
				  std::vector<uint32_t> iVertices):
		nTriangles(nTriangle),
		nVertices(nVertex),
		iVertices(std::move(iVertices))
	{ }

	uint32_t nTriangles, nVertices;
	std::vector<uint32_t> iVertices;
	std::vector<DPoint2f> uvs;
	std::vector<DVector3f> tangents;
	std::vector<DNormal3f> normals;
	std::vector<DPoint3f> positions;

	DECL_MANAGED_DENSE_COMP_DATA(CTriangleMesh, 16)
};

class JobTriangleMeshGenerateTriangles: public JobParallaziblePerCompGroup<CTriangleMesh, CMaterialHandle>
{
	void update(WECS* ecs, HEntity hEntity, CTriangleMesh& mesh, CMaterialHandle& hMaterial);
};

class STriangleMesh
{
	using cmp_type_list = typename wecs::type_list<CTriangleMesh,   CTransform>;
public:
	static uint32_t create(CTransform world,
						   CTriangleMesh mesh);


	static void generateTriangles(WECS& engine,
			    HEntity hEntity,
				const CTriangleMesh& triangleMesh,
				 CMaterialHandle hMaterial);


};

WPBR_END

