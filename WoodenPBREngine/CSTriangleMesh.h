#pragma once

#include "pch.h"
#include "CTransform.h"
#include "MEngine.h"
#include "WoodenMathLibrarry/DPoint.h"

WPBR_BEGIN

struct CTriangleMeshHandle: public HEntity
{
	CTriangleMeshHandle(HEntity h): HEntity(h){ }

	DECL_MANAGED_DENSE_COMP_DATA(CTriangleMeshHandle, 16)
}; 
struct CTriangleMesh
{
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

class STriangleMesh
{
	using cmp_type_list = typename wecs::type_list<CTriangleMesh, CTransform>;
public:
	static uint32_t create(CTransform world,
						   CTriangleMesh mesh);


	void generateTriangles(MEngine& engine,
			    HEntity hEntity,
				const CTriangleMesh& triangleMesh);


};

WPBR_END

