#pragma once

#include "pch.h"
#include "CSpectrum.h"
#include "CBXDF.h"
#include "CBSDF.h"
#include "CSSampler.h"
#include "SScattering.h"
#include "MicrofaceDistr.h"

WPBR_BEGIN


namespace SBSDFConductorMicroface
{
	float pdf(
		const CReflectDirSamplerMicroface& microface,
		const DVector3f& wo, const DVector3f& wi);

	Spectrum f_impl(
		const CSpectrumScale& R,
		const CFresnelConductor& fresnel,
		float whD,
		const DVector3f& wh,
		const DVector3f& wo,
		const DVector3f& wi
	);

	Spectrum f(
		const CReflectDirSamplerMicroface& microface,
		const CSpectrumScale& R,
		const CFresnelConductor& fresnel,
		const DVector3f& wo,
		const DVector3f& wi,
		float& wiPDF
	);


	Spectrum sample_f(
		const CReflectDirSamplerMicroface& microface,
		const CSpectrumScale& R,
		const CFresnelConductor& fresnel,
		const DVector3f& wo,
		const DPoint2f& u,
		DVector3f& wi,
		float& wiPDF);
}
	

class JobBSDFConductorMicrofaceCompute: 
	public JobParallaziblePerCompGroup<CReflectDirSamplerMicroface, CFresnelConductor, CBSDFComputeRequest, CSpectrumScale>
{
	void update(WECS* ecs, HEntity hEntity, CReflectDirSamplerMicroface& microface, CFresnelConductor& coductor,
				CBSDFComputeRequest& request, CSpectrumScale& R) final;
};

class JobBSDFConductorMicrofaceSample :
	public JobParallaziblePerCompGroup<CReflectDirSamplerMicroface, CFresnelConductor, CBSDFSampleRequest, CSpectrumScale>
{
	void update(WECS* ecs, HEntity hEntity, CReflectDirSamplerMicroface& microface, CFresnelConductor& coductor,
				CBSDFSampleRequest& request, CSpectrumScale& R) final;
};


WPBR_END