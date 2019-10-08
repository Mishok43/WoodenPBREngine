#pragma once

#include "pch.h"
#include "CSpectrum.h"
#include "CBSDF.h"
#include "CBXDF.h"
#include "SScattering.h"

WPBR_BEGIN


struct CBXDFSpecularReflection: public CompDummy
{
	DECL_MANAGED_DENSE_COMP_DATA(CBXDFSpecularReflection, 1)
};

class JobBSDFSpecularReflectionCompute :
	public JobParallaziblePerCompGroup<CBXDFSpecularReflection, CBSDFTransform, CFresnelDielectric, CBSDFComputeRequest, CSpectrumScale>
{
	void update(WECS* ecs, HEntity hEntity, CBXDFSpecularReflection&, CBSDFTransform& world, CFresnelDielectric& coductor,
				CBSDFComputeRequest& request, CSpectrumScale& R) final;
};

class JobBSDFSpecularReflectionSample :
	public JobParallaziblePerCompGroup<CBXDFSpecularReflection, CBSDFTransform, CFresnelDielectric, CBSDFSampleRequest, CSpectrumScale>
{
	void update(WECS* ecs, HEntity hEntity, CBXDFSpecularReflection& microface, CBSDFTransform& world, CFresnelDielectric& coductor,
				CBSDFSampleRequest& request, CSpectrumScale& R) final;
};

class SBXDFSpecularReflectance
{
public:
	static HEntity create(float etaI, float etaT, Spectrum R);

	static Spectrum f_impl(
		const CSpectrumScale& R,
		const CFresnelDielectric& fresnel,
		const DVector3f& wi
	);

	static Spectrum f(
		float& wiPDF
	);

	static Spectrum sample_f(
		const CSpectrumScale& R,
		const CFresnelDielectric& fresnel,
		const DVector3f& wo,
		DVector3f& wi,
		float& wiPDF);
};


WPBR_END

