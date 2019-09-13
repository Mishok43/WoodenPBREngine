#pragma once
#include "pch.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/Utils.h"
#include "CTransform.h"
#include "CSufraceInteraction.h"

WPBR_BEGIN


struct CTextureMappedPoint
{
	DPoint2f p;
	DVector2f dstdx;
	DVector2f dstdy;
};

struct CTextureMappingUV
{
	const float su, sv, du, dv;
	DECL_MANAGED_DENSE_COMP_DATA(CTextureMappingUV, 16)
}; DECL_OUT_COMP_DATA(CTextureMappingUV)

struct CTextureMappingSphere
{
	CTransform texture;
	DECL_MANAGED_DENSE_COMP_DATA(CTextureMappingSphere, 16)
}; DECL_OUT_COMP_DATA(CTextureMappingSphere)

class JobMapUV
{
	void map(
		WECS* ecs,
		HEntity hEntity,
		const CSurfaceInteraction& si,
		const CTextureMappingUV& mapping)
	{
		CTextureMappedPoint mappedPoint;
		mappedPoint.dstdx = DVector2f(mapping.su*si.dudx, mapping.sv*si.dvdx);
		mappedPoint.dstdy = DVector2f(mapping.su*si.dudy, mapping.sv*si.dvdy);
		mappedPoint.p = DPoint2f(mapping.su*si.uv[0] + mapping.du, mapping.sv*si.uv[1] + mapping.dv);
		ecs->addComponent<CTextureMappedPoint>(hEntity);
	}
};

class JobMapSphere
{
	DPoint2f sphere(const DPoint3f& p) const
	{
		DVector3f vec = normalize(texture(p) - DPoint3f(0.0, 0.0, 0.0));
		float theta = sphericalTheta(vec), phi = sphericalPhi(vec);
		return DPoint2f(theta*1.0 / PI, phi / (2 * PI));
	}

	void map(
		WECS* ecs,
		HEntity hEntity,
		const CSurfaceInteraction& si,
		const CTextureMappingSphere& mapping)
	{
		CTextureMappedPoint mappedPoint;

		DPoint2f st = sphere(si.p);

		const float dt = 0.1f;
		DPoint2f stdx = sphere(si.p + si.dpdx*dt);
		DVector2f dstdx = (stdx - st) / dt;
		DPoint2f stdy = sphere(si.p + si.dpdy*dt);
		DVector2f dstdy = (stdy - st) / dt;

		if (dstdx[1] > .5)        dstdx[1] = 1 - dstdx[1];
		else if (dstdx[1] < -.5f) dstdx[1] = -dstdx[1] + 1;
		if (dstdy[1] > .5)        dstdy[1] = 1 - dstdy[1];
		else if (dstdy[1] < -.5f) dstdy[1] = -dstdy[1] + 1;
		
		mappedPoint.dstdx = dstdx;
		mappedPoint.dstdy = dstdy;
		mappedPoint.p = st;

		ecs->addComponent<CTextureMappedPoint>(hEntity, mappedPoint);
	}
};


WPBR_END