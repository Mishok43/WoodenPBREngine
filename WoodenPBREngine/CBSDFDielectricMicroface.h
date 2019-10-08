#pragma once

#include "pch.h"
#include "CSpectrum.h"
#include "CBXDF.h"
#include "CBSDF.h"
#include "CSSampler.h"
#include "SScattering.h"
#include "MicrofaceDistr.h"

WPBR_BEGIN


namespace SBSDFDielectricMicroface
{
	float pdf(
		const CReflectDirSamplerMicroface& microface,
		const DVector3f& wo, const DVector3f& wi);

	Spectrum f_impl(
		const CSpectrumScale& R,
		const CFresnelDielectric& fresnel,
		float whD,
		const DVector3f& wh,
		const DVector3f& wo,
		const DVector3f& wi
	);

	Spectrum f(
		const CReflectDirSamplerMicroface& microface,
		const CSpectrumScale& R,
		const CFresnelDielectric& fresnel,
		const DVector3f& wo,
		const DVector3f& wi,
		float& wiPDF
	);


	Spectrum sample_f(
		const CReflectDirSamplerMicroface& microface,
		const CSpectrumScale& R,
		const CFresnelDielectric& fresnel,
		const DVector3f& wo,
		const DPoint2f& u,
		DVector3f& wi,
		float& wiPDF);
}


class JobBSDFDielectricMicrofaceCompute :
	public JobParallaziblePerCompGroup<CReflectDirSamplerMicroface, CBSDFTransform, CFresnelDielectric, CBSDFComputeRequest, CSpectrumScale>
{
	void update(WECS* ecs, HEntity hEntity, CReflectDirSamplerMicroface& microface, CBSDFTransform& world, CFresnelDielectric& coductor,
				CBSDFComputeRequest& request, CSpectrumScale& R) final;
};

class JobBSDFDielectricMicrofaceSample :
	public JobParallaziblePerCompGroup<CReflectDirSamplerMicroface, CBSDFTransform, CFresnelDielectric, CBSDFSampleRequest, CSpectrumScale>
{
	void update(WECS* ecs, HEntity hEntity, CReflectDirSamplerMicroface& microface, CBSDFTransform &world, CFresnelDielectric& coductor,
				CBSDFSampleRequest& request, CSpectrumScale& R) final;
};


WPBR_END