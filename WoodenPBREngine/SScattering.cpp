#include "pch.h"
#include "SScattering.h"
#include "CTextureBase.h"
#include "CTextureMapping.h"
#include "CBXDF.h"
#include "CBSDFSpecularReflectance.h"
#include "MEngine.h"
#include <iostream>

WPBR_BEGIN
DECL_OUT_COMP_DATA(CSampledLight)
DECL_OUT_COMP_DATA(CSampledWI)
DECL_OUT_COMP_DATA(CSampledLightLI)
DECL_OUT_COMP_DATA(CSampledLightPDF)


#define BSDF_SAMPLING false
#define LIGHT_SAMPLING false

void JobScatteringRequestEmittedLight::update(WECS* ecs)
{
	ComponentsGroup<CInteraction> sis = queryComponentsGroup<CInteraction>();
	std::vector<HEntity> deleteList;

	const MEngine& engine = MEngine::getInstance();
	HEntity hEnvLight = engine.getEnvLight();
	CLightLeComputeRequests& leComputeRequests = hEnvLight.getComponent<CLightLeComputeRequests>();

	for_each([&](HEntity hEntity, CInteraction& inter)
	{
		
		leComputeRequests.data.push_back(hEntity);

	/*	if (si.hCollision.h == INVALID_HANDLE)
		{
			deleteList.push_back(hEntity);
			return;
		}

		sp += 

		float factor = 5000000.f;
*/
		/*if (si.hCollision.hasComponent<CLight>())
		{
			sp += Spectrum(1.0f)*factor;
			deleteList.push_back(hEntity);
		}*/
	}, sis);

/*
	for (uint32_t i = 0; i < deleteList.size(); i++)
	{
		ecs->removeComponent<CSurfaceInteraction>(deleteList[i]);
	}*/

}

void JobScatteringAccumulateEmittedLight::update(WECS* ecs)
{
	ComponentsGroup<CSpectrum, CLightLE> sis = queryComponentsGroup<CSpectrum, CLightLE>();
//	std::vector<HEntity> deleteList;

	for_each([&](HEntity hEntity, CSpectrum& sp, CLightLE& le)
	{
		sp += le.L;
		//deleteList.push_back(hEntity);
	}, sis);
	//
	//for (uint32_t i = 0; i < deleteList.size(); i++)
	//{
	//	ecs->removeComponent<CSurfaceInteraction>(deleteList[i]);
	//}
}

void JobScatteringAccumulateEmittedLight::finish(WECS* ecs)
{
	ecs->clearComponents<CInteraction>();
	ecs->clearComponents<CLightLE>();
}

void JobScatteringSampleLight::update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si, CSamples1D& samples)
{
	ComponentsGroup<CLight> lights = queryComponentsGroup<CLight>();
	uint32_t u = (samples.data[samples.i++]+0.5f) * (lights.size() - 1);
	HEntity hLight = lights.getEntity(u);
	ecs->addComponent<CSampledLight>(hEntity, CSampledLight{ hLight });
}



void JobScatteringSampleLightLI::update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si, CSampledLight& sl)
{
	CLightLiSampleRequests& requests = ecs->getComponent<CLightLiSampleRequests>(sl.h);
	requests.data.push_back(hEntity);
}


void JobScatteringCastShadowRays::update(WECS* ecs, HEntity hEntity,
										 CSurfaceInteraction& si, CSampledWI& wi)
{
	CRayCast rayCast;
	rayCast.bSurfInteraction = false;
	rayCast.ray.dir = wi;
	rayCast.ray.origin = si.p+ rayCast.ray.dir*0.1;
	ecs->addComponent<CRayCast>(hEntity, std::move(rayCast));
}

void JobScatteringProcessShadowRay::update(WECS* ecs, HEntity hEntity,
										   CSampledLight& si, CInteraction& sl, CSampledBSDF& bsdf)
{
	if (sl.hCollision.h == INVALID_HANDLE || sl.hCollision == si.h)
	{
		ecs->addComponent<CBSDFComputeRequest>(bsdf.h, CBSDFComputeRequest{ hEntity });
	}
	else
	{
		int a;
	}
}

void JobScatteringProcessShadowRay::finish(WECS* ecs)
{
	ecs->clearComponents<CInteraction>();
}

void JobScatteringIntegrateImportanceLight::update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si, CSampledLightLI& li, CSampledLightPDF& liPDF,
												   CSampledBSDFValue& bsdf, CSampledBSDFPDF& bsdfPDF, CSampledWI& wi, CSpectrum& accumulatedLI)
{
	if (li.isBlack())
	{
		return;
	}

	if (bsdfPDF.p == 0.0)
	{
		return;
	}

	float weight = PowerHeuristic(1, liPDF.p, 1, bsdfPDF.p);
#if LIGHT_SAMPLING
	weight = 1.0f;
#endif

#if BSDF_SAMPLING
	weight = 0.0f;
#endif

	accumulatedLI += li * bsdf*(weight / liPDF.p);
}

void JobScatteringIntegrateImportanceLight::finish(WECS* ecs)
{
	ecs->clearComponents<CSampledLightLI>();
	ecs->clearComponents<CSampledLightPDF>();
	ecs->clearComponents<CBSDFComputeRequest>();
	ecs->clearComponents<CBSDFSampleRequest>();
	ecs->clearComponents<CSampledBSDFValue>();
	ecs->clearComponents<CSampledBSDFPDF>();
	ecs->clearComponents<CSampledWI>();
}

inline float JobScatteringIntegrateImportanceLight::PowerHeuristic(int nf, float fPdf, int ng, float gPdf)
{
	float f = nf * fPdf, g = ng * gPdf;
	return (f * f) / (f * f + g * g);
}



void JobScatteringSampleBSDF::update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si, CSampledBSDF& sbsdf)
{
	ecs->addComponent<CBSDFSampleRequest>(sbsdf.h, CBSDFSampleRequest{ hEntity });
}


void JobScatteringCastShadowRaysWithInteraction::update(WECS* ecs, HEntity hEntity,
														CSurfaceInteraction& si, CSampledWI& wi)
{
	CRayCast rayCast;
	rayCast.bSurfInteraction = false;
	rayCast.ray.dir = wi;
	rayCast.ray.origin = si.p+ rayCast.ray.dir*0.01;
	ecs->addComponent<CRayCast>(hEntity, std::move(rayCast));
}



void JobScatteringProcessShadowRayWithInteraction::update(WECS* ecs, HEntity hEntity,
														  CSampledLight& si, CInteraction& sl)
{
	if (sl.hCollision.h == INVALID_HANDLE || sl.hCollision == si.h)
	{
		CLightLiComputeRequests& requests = ecs->getComponent<CLightLiComputeRequests>(si.h);
		requests.data.push_back(hEntity);
	}
	else
	{
		int a = 1;
	}
}

void JobScatteringProcessShadowRayWithInteraction::finish(WECS* ecs)
{
	ecs->clearComponents<CInteraction>();
}


void JobScatteringIntegrateImportanceBSDF::update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si, CSampledLightLI& li, CSampledLightPDF& liPDF,
												  CSampledBSDFValue& bsdf, CSampledBSDFPDF& bsdfPDF, CSampledWI& wi, CSpectrum& accumulatedLI)
{
	if (li.isBlack())
	{
		return;
	}


	float weight;
	if (bsdfPDF.p == 1.0f)
	{
		weight = 1.0f;
	}
	else
	{
		weight = PowerHeuristic(1, bsdfPDF.p, 1, liPDF.p);
	}

#if BSDF_SAMPLING
	weight = 1.0f;
#endif

#if LIGHT_SAMPLING
	weight = 0.0f;
#endif
	accumulatedLI += li * bsdf*(weight / bsdfPDF.p);
}

void JobScatteringIntegrateImportanceBSDF::finish(WECS* ecs)
{

	ecs->clearComponents<CSampledLightLI>();
	ecs->clearComponents<CSampledLightPDF>();
	ecs->clearComponents<CBSDFComputeRequest>();
	ecs->clearComponents<CBSDFSampleRequest>();
	ecs->clearComponents<CSampledBSDFValue>();
	ecs->clearComponents<CSampledBSDFPDF>();
	ecs->clearComponents<CSampledWI>();
}

inline float JobScatteringIntegrateImportanceBSDF::PowerHeuristic(int nf, float fPdf, int ng, float gPdf)
{
	float f = nf * fPdf, g = ng * gPdf;
	return (f * f) / (f * f + g * g);
}


void JobScatteringFinish::update(WECS* ecs)
{

}
void JobScatteringFinish::finish(WECS* ecs)
{
	ecs->clearComponents<CSurfaceInteraction>();
	ecs->clearComponents<CSampledLight>();
	ecs->clearComponents<CRayDifferential>();
	ecs->clearComponents<CTextureMappedPoint>();
	ecs->clearComponents<CSampledBSDF>();
	ecs->clearComponents<CBSDFTransform>();
	ecs->deleteEntitiesOfComponents<CReflectDirSamplerMicroface>();
	ecs->deleteEntitiesOfComponents<CBXDFSpecularReflection>();
	ecs->clearComponents<CBXDFSpecularReflection>();
	ecs->clearComponents<CReflectDirSamplerMicroface>();
	ecs->clearComponents<CSpectrumScale>();
	ecs->clearComponents<CFresnelConductor>();
	ecs->clearComponents<CFresnelDielectric>();
}


WPBR_END

