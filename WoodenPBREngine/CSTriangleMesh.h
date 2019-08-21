#pragma once

#include "pch.h"
#include "CTransform.h"
#include "WoodenMathLibrarry/DPoint.h"

WPBR_BEGIN

struct CPositionArray
{
	std::vector<DPoint3f> data;
	DECL_UNMANAGED_DENSE_COMP_DATA(CPositionArray, 16)
}; DECL_OUT_COMP_DATA(CPositionArray)

struct CNormalArray
{
	std::vector<DNormal3f> data;
	DECL_UNMANAGED_DENSE_COMP_DATA(CNormalArray, 16)
}; DECL_OUT_COMP_DATA(CNormalArray)

struct CTangentArray
{
	std::vector<DVector3f> data;
	DECL_UNMANAGED_DENSE_COMP_DATA(CTangentArray, 16)
}; DECL_OUT_COMP_DATA(CTangentArray)

struct CUVArray
{
	std::vector<DPoint2f> data;
	DECL_UNMANAGED_DENSE_COMP_DATA(CUVArray, 16)
}; DECL_OUT_COMP_DATA(CUVArray)

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

	DECL_UNMANAGED_DENSE_COMP_DATA(CTriangleMesh, 16)
}; DECL_OUT_COMP_DATA(CTriangleMesh)

class STriangleMesh
{
	using cmp_type_list = typename wecs::type_list<CTriangleMesh, CTransform, CPositionArray>;
public:
	static uint32_t create(CTransform world,
						   CTriangleMesh mesh,
						   CPositionArray positions,
						   CNormalArray* normals = nullptr,
						   CTangentArray* tangents = nullptr,
						   CUVArray* uvs = nullptr);


};

WPBR_END

