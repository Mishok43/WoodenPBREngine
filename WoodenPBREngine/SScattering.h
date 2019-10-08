#pragma once

#include "pch.h"
#include "WoodenECS/Job.h"
#include "WoodenMathLibrarry/DVector.h"
#include "CBSDF.h"
#include "CLight.h"
#include "CSufraceInteraction.h"
#include "CSampler.h"
WPBR_BEGIN

struct CSampledLight
{
	HEntity h;

	DECL_MANAGED_DENSE_COMP_DATA(CSampledLight, 16)
};


struct CSampledWI: public DVector3f
{
	CSampledWI() = default;

	CSampledWI(DVector3f v):
		DVector3f(std::move(v)) {}

	DECL_MANAGED_DENSE_COMP_DATA(CSampledWI, 16)
};

struct CSampledLightLI: public Spectrum
{
	CSampledLightLI() = default;

	CSampledLightLI(Spectrum s):
		Spectrum(std::move(s)) {}

	DECL_MANAGED_DENSE_COMP_DATA(CSampledLightLI, 16)
};

struct CSampledLightPDF
{
	float p;
	DECL_MANAGED_DENSE_COMP_DATA(CSampledLightPDF, 16)
};


class JobScatteringAccumulateEmittedLight: public Job
{
	void update(WECS* ecs) override;
	void finish(WECS* ecs) override;
};

class JobScatteringSampleLight : public JobParallaziblePerCompGroup<CSurfaceInteraction, CSamples1D>
{
	void update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si, CSamples1D& samples) override;
};

class JobScatteringSampleLightLI: public JobParallaziblePerCompGroup<CSurfaceInteraction, CSampledLight>
{
	void update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si, CSampledLight& sl) final;
};

class JobScatteringCastShadowRays : public 
	JobParallaziblePerCompGroup<CSurfaceInteraction, CSampledWI>
{
	void update(WECS* ecs, HEntity hEntity,
				CSurfaceInteraction& si, CSampledWI& wi) final;
};

class JobScatteringProcessShadowRay :
	public JobParallaziblePerCompGroup<CSampledLight, CInteraction, CSampledBSDF>
{
	void update(WECS* ecs, HEntity hEntity,
				CSampledLight& si, CInteraction& sl, CSampledBSDF& bsdf) final;
	void finish(WECS* ecs) final;
};

class JobScatteringIntegrateImportanceLight : 
	public JobParallaziblePerCompGroup<CSurfaceInteraction, CSampledLightLI, CSampledLightPDF, CSampledBSDFValue, CSampledBSDFPDF, CSampledWI, CSpectrum>
{
	void update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si, CSampledLightLI& li, CSampledLightPDF& liPDF,
				CSampledBSDFValue& bsdf, CSampledBSDFPDF& bsdfPDF, CSampledWI& wi, CSpectrum& accumulatedLI) final;

	void finish(WECS* ecs) final;
	inline float PowerHeuristic(int nf, float fPdf, int ng, float gPdf);
};

class JobScatteringSampleBSDF : public JobParallaziblePerCompGroup<CSurfaceInteraction, CSampledBSDF>
{
	void update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si, CSampledBSDF& sbsdf) final;
};


class JobScatteringCastShadowRaysWithInteraction : public
	JobParallaziblePerCompGroup<CSurfaceInteraction, CSampledWI>
{
	void update(WECS* ecs, HEntity hEntity,
				CSurfaceInteraction& si, CSampledWI& wi) final;
};


class JobScatteringProcessShadowRayWithInteraction :
	public JobParallaziblePerCompGroup<CSampledLight, CInteraction>
{
	void update(WECS* ecs, HEntity hEntity,
				CSampledLight& si, CInteraction& sl) final;

	void finish(WECS* ecs) final;
};

class JobScatteringIntegrateImportanceBSDF :
	public JobParallaziblePerCompGroup<CSurfaceInteraction, CSampledLightLI, CSampledLightPDF, CSampledBSDFValue, CSampledBSDFPDF, CSampledWI, CSpectrum>
{
	void update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si, CSampledLightLI& li, CSampledLightPDF& liPDF,
				CSampledBSDFValue& bsdf, CSampledBSDFPDF& bsdfPDF, CSampledWI& wi, CSpectrum& accumulatedLI) final;

	void finish(WECS* ecs) final;

	inline float PowerHeuristic(int nf, float fPdf, int ng, float gPdf);
};

class JobScatteringFinish: public Job
{
	void update(WECS* ecs) override;
	void finish(WECS* ecs) override;
};

WPBR_END