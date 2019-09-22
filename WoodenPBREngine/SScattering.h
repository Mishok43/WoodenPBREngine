#pragma once

#include "pch.h"
#include "WoodenECS/Job.h"
#include "WoodenMathLibrarry/DVector.h"
#include "CBSDF.h"
#include "CLight.h"
WPBR_BEGIN

struct CSampledLight
{
	HEntity h;
};

struct CSampledWI: public DVector3f
{
	CSampledWI() = default;

	CSampledWI(DVector3f v):
		DVector3f(std::move(v)) {}

	DECL_MANAGED_DENSE_COMP_DATA(CSampledWI, 16)
}; DECL_OUT_COMP_DATA(CSampledWI)

struct CSampledLightLI: public Spectrum
{
	CSampledLightLI() = default;

	CSampledLightLI(Spectrum s):
		Spectrum(std::move(s)) {}

	DECL_MANAGED_DENSE_COMP_DATA(CSampledLightLI, 16)
}; DECL_OUT_COMP_DATA(CSampledLightLI)

struct CSampledLightPDF
{
	float p;
};

struct CSampledBSDF
{
	CSampledBSDF(HEntity h) : h(h)
	{
	};
	HEntity h;

	DECL_MANAGED_DENSE_COMP_DATA(CSampledBSDF, 16)
}; DECL_OUT_COMP_DATA(CSampledBSDF)

class JobScatteringAccumulateEmittedLight: public Job
{
	void update(WECS* ecs)
	{
		ComponentsGroup<CSurfaceInteraction, CSpectrum> sis = queryComponentsGroup<CSurfaceInteraction, CSpectrum>();
		std::vector<HEntity> deleteList;
		for_each([&](HEntity hEntity, CSurfaceInteraction& si, CSpectrum& sp)
		{
			if (si.hCollision.hasComponent<CLight>())
			{
				sp += si.hCollision.getComponent<CLight>().LEmit;
			}
			deleteList.push_back(hEntity);
		}, sis);

		for (uint32_t i = 0; i < deleteList.size(); i++)
		{
			ecs->removeComponent<CSurfaceInteraction>(deleteList[i]);
		}
	}
};

class JobScatteringSampleLight : public JobParallaziblePerCompGroup<CSurfaceInteraction>
{
	void update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si, CSamples1D& samples)
	{
		ComponentsGroup<CLight> lights = queryComponentsGroup<CLight>();
		uint32_t u = samples.data[samples.i++]* (lights.size() - 1);
		HEntity hLight = lights.getEntity(u);
		ecs->addComponent<CSampledLight>(hEntity, CSampledLight{ hLight });
	}
};

class JobScatteringSampleLightLI: public JobParallaziblePerCompGroup<CSurfaceInteraction, CSampledLight>
{
	void update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si, CSampledLight& sl) final
	{
		CLightSamplingRequests& requests = ecs->getComponent<CLightSamplingRequests>(sl.h);
		requests.data.push_back(hEntity);
	}
};

class JobScatteringCastShadowRays : public 
	JobParallaziblePerCompGroup<CSurfaceInteraction, CSampledWI>
{
	void update(WECS* ecs, HEntity hEntity,
				CSurfaceInteraction& si, CSampledWI& wi) final
	{
		CRayCast rayCast;
		rayCast.bSurfInteraction = false;
		rayCast.ray.origin = si.p;
		rayCast.ray.dir = wi;
		ecs->addComponent<CRayCast>(hEntity, std::move(rayCast));
	}
};

class JobScatteringProcessShadowRay :
	public JobParallaziblePerCompGroup<CSampledLight, CRayCast, CSampledBSDF>
{
	void update(WECS* ecs, HEntity hEntity,
				CSampledLight& si, CRayCast& sl, CSampledBSDF& bsdf) final
	{
		if (sl.hCollision == si.h)
		{
			ecs->addComponent<CBSDFComputeRequest>(bsdf.h, CBSDFComputeRequest{ hEntity });
		}	
	}

	void finish(WECS* ecs) final
	{
		ecs->clearComponents<CRayCast>();
		ecs->clearComponents<CInteraction>();
	}
};

class JobScatteringIntegrateImportanceLight : 
	public JobParallaziblePerCompGroup<CSampledLightLI, CSampledLightPDF, CSampledBSDFValue, CSampledBSDFPDF, CSpectrum>
{
	void update(WECS* ecs, HEntity hEntity, CSampledLightLI& li, CSampledLightPDF& liPDF, 
				CSampledBSDFValue& bsdf, CSampledBSDFPDF& bsdfPDF, CSpectrum& accumulatedLI) final
	{
		if (li.isBlack())
		{
			return;
		}

		float weight = PowerHeuristic(1,  liPDF.p, 1, bsdfPDF.p);
		accumulatedLI += li * bsdf*(weight / liPDF.p);
	}

	void finish(WECS* ecs) final
	{
		ecs->clearComponents<CSampledLightLI>();
		ecs->clearComponents<CSampledLightPDF>();
		ecs->clearComponents<CBSDFComputeRequest>();
		ecs->clearComponents<CSampledBSDFValue>();
		ecs->clearComponents<CSampledBSDFPDF>();
		ecs->clearComponents<CSampledWI>();
	}

	inline float PowerHeuristic(int nf, float fPdf, int ng, float gPdf)
	{
		float f = nf * fPdf, g = ng * gPdf;
		return (f * f) / (f * f + g * g);
	}
};

class JobScatteringSampleBSDF : public JobParallaziblePerCompGroup<CSurfaceInteraction, CSampledBSDF>
{
	void update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si, CSampledBSDF& sbsdf) final
	{
		ecs->addComponent<CBSDFSampleRequest>(sbsdf.h, CBSDFSampleRequest{ hEntity });
	}
};


class JobScatteringCastShadowRaysWithInteraction : public
	JobParallaziblePerCompGroup<CSurfaceInteraction, CSampledWI>
{
	void update(WECS* ecs, HEntity hEntity,
				CSurfaceInteraction& si, CSampledWI& wi) final
	{
		CRayCast rayCast;
		rayCast.bSurfInteraction = false;
		rayCast.ray.origin = si.p;
		rayCast.ray.dir = wi;
		ecs->addComponent<CRayCast>(hEntity, std::move(rayCast));
	}
};


class JobScatteringProcessShadowRayWithInteraction :
	public JobParallaziblePerCompGroup<CSampledLight, CRayCast>
{
	void update(WECS* ecs, HEntity hEntity,
				CSampledLight& si, CRayCast& sl) final
	{
		if (sl.hCollision == si.h)
		{
			CLightComputeRequests& requests = ecs->getComponent<CLightComputeRequests>(si.h);
			requests.data.push_back(hEntity);
		}
	}

	void finish(WECS* ecs) final
	{
		ecs->clearComponents<CRayCast>();
	}
};

class JobScatteringIntegrateImportanceBSDF :
	public JobParallaziblePerCompGroup<CSampledLightLI, CSampledLightPDF, CSampledBSDFValue, CSampledBSDFPDF, CSpectrum>
{
	void update(WECS* ecs, HEntity hEntity, CSampledLightLI& li, CSampledLightPDF& liPDF,
				CSampledBSDFValue& bsdf, CSampledBSDFPDF& bsdfPDF, CSpectrum& accumulatedLI) final
	{
		if (li.isBlack())
		{
			return;
		}

		float weight = PowerHeuristic(1, bsdfPDF.p, 1, liPDF.p);
		accumulatedLI += li * bsdf*(weight / bsdfPDF.p);
	}

	void finish(WECS* ecs) final
	{
		ecs->clearComponents<CSampledLightLI>();
		ecs->clearComponents<CSampledLightPDF>();
		ecs->clearComponents<CBSDFComputeRequest>();
		ecs->clearComponents<CSampledBSDFValue>();
		ecs->clearComponents<CSampledBSDFPDF>();
		ecs->clearComponents<CSampledWI>();
	}

	inline float PowerHeuristic(int nf, float fPdf, int ng, float gPdf)
	{
		float f = nf * fPdf, g = ng * gPdf;
		return (f * f) / (f * f + g * g);
	}
};

WPBR_END