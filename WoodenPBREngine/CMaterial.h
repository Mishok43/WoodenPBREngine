#pragma once
#include "pch.h"
#include "CRayDifferential.h"
#include "CSufraceInteraction.h"
#include "CTexture.h"
#include "WoodenECS/Job.h"

WPBR_BEGIN

struct CBSDFRequests
{
	std::vector<HEntity, AllocatorAligned<HEntity>> data;
	DECL_MANAGED_DENSE_COMP_DATA(CBSDFRequests, 16)
}; DECL_OUT_COMP_DATA(CBSDFRequests)

class JobGenerateBSDFRequests: public JobParallaziblePerCompGroup<CSurfaceInteraction>
{
	void update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& comps)
	{
		CBSDFRequests& requests =  ecs->getComponent<CBSDFRequests>(comps.hCollision);
		requests.data.push_back(hEntity);
	}
};

void bump(const CTextureBumpMap& bumpMap, CSurfaceInteraction& surfInter)
{
	CSurfaceInteraction siTmp = surfInter;
	float du = 0.5f*(std::abs(surfInter.dudx) + std::abs(surfInter.dudy));
	if (du == 0.0f)
	{
		du = 0.01f;
	}
	siTmp.p = surfInter.p + surfInter.shading.dpdu*du;
	siTmp.uv = surfInter.uv + DVector2f(du, 0.0f);
	siTmp.n = normalize((DNormal3f)cross(surfInter.shading.dpdu, surfInter.shading.dpdv)) + 
		surfInter.shading.dndu*du;
	float uDisplace = bumpMap.sample(siTmp);

	float dv = 0.5f*(std::abs(surfInter.dvdx) + std::abs(surfInter.dvdy));
	if (dv == 0.0f)
	{
		dv = 0.01f;
	}

	siTmp.p = surfInter.p + surfInter.shading.dpdv*dv;
	siTmp.uv = surfInter.uv + DVector2f(0.0f, dv);
	siTmp.n = normalize((DNormal3f)cross(surfInter.shading.dpdu, surfInter.shading.dpdv)) +
		surfInter.shading.dndv*dv;
	float vDisplace = bumpMap.sample(siTmp);
	float displace = bumpMap.sample(surfInter);

	DVector3f dpdu = surfInter.shading.dpdu +
		DVector3f(surfInter.shading.n)*(uDisplace - displace) / du +
		DVector3f(surfInter.shading.dndu)*displace;
	DVector3f dpdv = surfInter.shading.dpdv +
		DVector3f(surfInter.shading.n)*(vDisplace - displace) / dv +
		DVector3f(surfInter.shading.dndv)*displace;

	surfInter.setShadingGeometry(dpdu, dpdv, surfInter.shading.dndu, surfInter.shading.dndv,
						   false);
}

WPBR_END

