#pragma once

#include "pch.h"
#include "CBounds.h"
#include "CCentroid.h"
#include "CSufraceInteraction.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/DNormal.h"
#include "CTextureMapping.h"
#include "WoodenMathLibrarry/DVector.h"
#include "WoodenECS/Job.h"
#include "CSTriangleMesh.h"
#include "MEngine.h"

WPBR_BEGIN


struct CInteractionTriangle
{
	DPoint3f barycentric;
	std::array<DVector3f, 3> posT;
	HEntity hTriangle;
	float tHit;
	DECL_MANAGED_DENSE_COMP_DATA(CInteractionTriangle, 16)
}; DECL_OUT_COMP_DATA(CInteractionTriangle)

struct CTriangle
{
	uint32_t index;

	DECL_MANAGED_DENSE_COMP_DATA(CTriangle, 16)
}; DECL_OUT_COMP_DATA(CTriangle) 


class JobProcessTriangleFullInteractionRequests : public JobParallazible
{
	constexpr static uint32_t slice = 64;

	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		ComponentsGroup<CInteractionTriangle> collisions = queryComponentsGroup<CInteractionTriangle>();
		return min(nWorkThreads, collisions.size<CInteractionTriangle>() / slice);
	}

	void update(WECS* ecs, uint8_t iThread) override;
};

class JobProcessTriangleInteractionRequests: public JobParallazible
{
	constexpr static uint32_t slice = 64;

	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		ComponentsGroup<CInteractionTriangle> collisions = queryComponentsGroup<CInteractionTriangle>();
		return min(nWorkThreads, collisions.size<CInteractionTriangle>()/ slice);
	}

	void update(WECS* ecs, uint8_t iThread) override;
};


class JobUpdateBoundsAndCentroidTriangle : public JobParallazible
{
	constexpr static uint32_t slice = 64;

	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		ComponentsGroup<CTriangle> collisions = queryComponentsGroup<CTriangle>();
		return min(nWorkThreads, collisions.size<CTriangle>() / slice);
	}

	void update(WECS* ecs, uint8_t iThread) override
	{

		uint32_t nCollisions = queryComponentsGroup<CTriangle>().size<CTriangle>();
		uint32_t sliceSize = (nCollisions + getNumThreads()-1) /getNumThreads();
		uint32_t iStart = iThread * sliceSize;

		ComponentsGroupSlice<CTriangle, CTriangleMesh, CBounds, CCentroid> triangles =
			queryComponentsGroupSlice<CTriangle, CTriangleMesh, CBounds, CCentroid>(Slice(iStart, sliceSize));

		for_each([](HEntity hEntity, const CTriangle& eTriangle, const CTriangleMesh& triangleMesh,
				 CBounds& eBounds, CCentroid& eCentroid)
		{
			std::array<uint32_t, 3> iVertex =
			{
				triangleMesh.iVertices[eTriangle.index * 3],
				triangleMesh.iVertices[eTriangle.index * 3 + 1],
				triangleMesh.iVertices[eTriangle.index * 3 + 2]
			};

			DBounds3f bounds = DBounds3f(triangleMesh.positions[iVertex[0]],
										 triangleMesh.positions[iVertex[1]]);
			eBounds = DBounds3f(bounds, triangleMesh.positions[iVertex[2]]);
			eCentroid = CCentroid(eBounds);
		}, triangles);
	}
};

class STriangle
{
public:
	static CTextureMappedPoint mapUV(
		const CSurfaceInteraction& si
	);
};


WPBR_END


