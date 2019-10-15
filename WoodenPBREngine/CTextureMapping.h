#pragma once
#include "pch.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/Utils.h"
#include "CTransform.h"
#include "CSufraceInteraction.h"

WPBR_BEGIN


struct CMapUVRequests
{
	std::vector<HEntity, AllocatorAligned2<HEntity>> si;
	DECL_MANAGED_DENSE_COMP_DATA(CMapUVRequests, 16)
}; 

struct CTextureMappedPoint
{
	DPoint2f p;
	DVector2f dstdx;
	DVector2f dstdy;
	DECL_MANAGED_DENSE_COMP_DATA(CTextureMappedPoint, 16)
};


class JobMapUVRequestsGenerate : public JobParallaziblePerCompGroup<CSurfaceInteraction>
{
	void update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si) override;
};

class TextureMappingUV
{
public:
	static CTextureMappedPoint map(
		const CSurfaceInteraction& si,
		const DVector2f& suv = DVector2f(1.0f, 1.0f),
		const DVector2f& duv = DVector2f(0.0f, 0.0f))
	{
		CTextureMappedPoint mappedPoint;
		mappedPoint.dstdx =  suv*DVector2f(si.dudx, si.dvdx) + duv;
		mappedPoint.dstdy = suv*DVector2f(si.dudy, si.dvdy)+duv;
		mappedPoint.p = suv*si.uv + duv;

		assert(mappedPoint.p.x() <= 1.0f);
		assert(mappedPoint.p.y() <= 1.0f);
		return mappedPoint;
	}
};

class TextureMappingSphere
{
public:
	static DPoint2f sphere(const DPoint3f& p, const CTransform& world) 
	{
		DVector3f vec = normalize(world(p, INV_TRANFORM) - DPoint3f(0.0, 0.0, 0.0));
		float theta = sphericalTheta(vec), phi = sphericalPhi(vec);
		return DPoint2f(theta*1.0 / PI, phi / (2 * PI));
	}

	static CTextureMappedPoint map(
		const CSurfaceInteraction& si,
		const CTransform& world)
	{
		CTextureMappedPoint mappedPoint;

		DPoint2f st = sphere(si.p, world);

		const float dt = 0.1f;
		DPoint2f stdx = sphere(si.p + si.dpdx*dt, world);
		DVector2f dstdx = (stdx - st) / dt;
		DPoint2f stdy = sphere(si.p + si.dpdy*dt, world);
		DVector2f dstdy = (stdy - st) / dt;

		if (dstdx[1] > .5)        dstdx[1] = 1 - dstdx[1];
		else if (dstdx[1] < -.5f) dstdx[1] = -dstdx[1] + 1;
		if (dstdy[1] > .5)        dstdy[1] = 1 - dstdy[1];
		else if (dstdy[1] < -.5f) dstdy[1] = -dstdy[1] + 1;

		mappedPoint.dstdx = dstdx;
		mappedPoint.dstdy = dstdy;
		mappedPoint.p = st;


		return mappedPoint;
	}
};


WPBR_END