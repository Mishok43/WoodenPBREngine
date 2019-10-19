#pragma once

#include "pch.h"
#include "CBounds.h"
#include "CCentroid.h"
#include "CSufraceInteraction.h"
#include "CTextureMapping.h"
#include "CSTriangleMesh.h"
#include "WoodenECS/Job.h"

WPBR_BEGIN


struct CInteractionTriangle
{
	DPoint3f barycentric;
	float tHit;
	DECL_MANAGED_DENSE_COMP_DATA(CInteractionTriangle, 16)
};

struct CTriangle
{
	uint32_t index;
	HEntity hMesh;

	DECL_MANAGED_DENSE_COMP_DATA(CTriangle, 16)
};


class JobProcessTriangleFullInteractionRequests : public JobParallaziblePerCompGroup<CTriangle, CInteractionFullRequests>
{

	void update(WECS* ecs, HEntity hEntity, CTriangle& triangle,
			 CInteractionFullRequests& requests) override;

	void finish(WECS* ecs) override;
};

class JobProcessTriangleInteractionRequests: public JobParallaziblePerCompGroup<CTriangle, CInteractionRequests>
{
	void update(WECS* ecs, HEntity hEntity, CTriangle& triangle, CInteractionRequests& requests) override;
};


class JobUpdateBoundsAndCentroidTriangle : public JobParallazible
{
	constexpr static uint32_t slice = 64;

	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		ComponentsGroup<CTriangle> collisions = queryComponentsGroup<CTriangle>();
		return min(nWorkThreads, collisions.size<CTriangle>() / slice);
	}

	void update(WECS* ecs, uint8_t iThread) override;
};

class JobTriangleProcessMapUVRequests: public JobParallaziblePerCompGroup<CTriangle, CMapUVRequests>
{
	void update(WECS* ecs, HEntity hEntity, CTriangle& triangle, CMapUVRequests& requests);
};

class STriangle
{
public:
	static CTextureMappedPoint mapUV(
		const CSurfaceInteraction& si
	);
};


WPBR_END


