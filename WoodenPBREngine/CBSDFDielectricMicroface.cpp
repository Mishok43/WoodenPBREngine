
#pragma once

#include "pch.h"
#include "CBSDFDielectricMicroface.h"

WPBR_BEGIN



float SBSDFDielectricMicroface::pdf(
	const CReflectDirSamplerMicroface& microface,
	const DVector3f& wo, const DVector3f& wi)
{
	DVector3f wh = normalize(wo + wi);
	return microface.pdf(wh) / (4 * dot(wo, wh));
}


Spectrum SBSDFDielectricMicroface::f_impl(
	const CSpectrumScale& R,
	const CFresnelDielectric& fresnel,
	float whD,
	const DVector3f& wh,
	const DVector3f& wo,
	const DVector3f& wi
)
{
	float cosThetaO = absCosTheta(wo), cosThetaI = absCosTheta(wi);
	if (cosThetaI == 0 || cosThetaO == 0)
	{
		return Spectrum(0.);
	}

	if (wh.x() == 0 && wh.y() == 0 && wh.z() == 0)
	{
		return Spectrum(0.);
	}

	float fr = fresnel.f(dot(wi, wh));
	return  R*(fr *whD / (4 * cosThetaI*cosThetaO));
}

Spectrum SBSDFDielectricMicroface::f(
	const CReflectDirSamplerMicroface& microface,
	const CSpectrumScale& R,
	const CFresnelDielectric& fresnel,
	const DVector3f& wo,
	const DVector3f& wi,
	float& wiPDF
)
{
	const DVector3f& wh = normalize(wo + wi);
	float whD = microface.d(wh);
	wiPDF = microface.pdf(wh, whD) / (4 * dot(wo, wh));

	return f_impl(R, fresnel, whD, wh, wo, wi);
}


Spectrum SBSDFDielectricMicroface::sample_f(
	const CReflectDirSamplerMicroface& microface,
	const CSpectrumScale& R,
	const CFresnelDielectric& fresnel,
	const DVector3f& wo,
	const DPoint2f& u,
	DVector3f& wi,
	float& wiPDF)
{
	DVector3f wh;

	float whD;
	wh = microface.sample_f(wo, u, whD);
	wi = reflect(wo, wh);
	if (!isSameHemisphere(wi, wo))
	{
		return Spectrum(0.0f);
	}

	wiPDF = microface.pdf(wh, whD) / (4 * dot(wo, wh));
	return f_impl(R, fresnel, whD, wh, wo, wi);
}

void JobBSDFDielectricMicrofaceCompute::update(WECS* ecs, HEntity hEntity, CReflectDirSamplerMicroface& microface, CBSDFTransform& world, CFresnelDielectric& coductor,
											  CBSDFComputeRequest& request, CSpectrumScale& R)
{
	const CSampledWI& sampledWI = ecs->getComponent<CSampledWI>(request.h);
	const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(request.h);

	DVector3f wiL = world(sampledWI);

	CSampledBSDFPDF pdf;
	CSampledBSDFValue bsdf = SBSDFDielectricMicroface::f(microface, R, coductor, world(si.wo), wiL, pdf.p);
	bsdf = Spectrum(bsdf * absCosTheta(wiL));
	ecs->addComponent<CSampledBSDFValue>(request.h, std::move(bsdf));
	ecs->addComponent<CSampledBSDFPDF>(request.h, std::move(pdf));
}

void JobBSDFDielectricMicrofaceSample::update(WECS* ecs, HEntity hEntity, CReflectDirSamplerMicroface& microface, CBSDFTransform& world, CFresnelDielectric& coductor,
											 CBSDFSampleRequest& request, CSpectrumScale& R)
{
	const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(request.h);
	CSamples2D& samples = ecs->getComponent<CSamples2D>(request.h);

	CSampledBSDFPDF pdf;
	CSampledWI wi;
	CSampledBSDFValue bsdf = SBSDFDielectricMicroface::sample_f(microface, R, coductor, world(si.wo),
															   samples.next(), wi, pdf.p);
	bsdf = Spectrum(bsdf*absCosTheta(wi));
	ecs->addComponent<CSampledBSDFValue>(request.h, std::move(bsdf));
	ecs->addComponent<CSampledBSDFPDF>(request.h, std::move(pdf));

	wi = world(wi, INV_TRANFORM);
	ecs->addComponent<CSampledWI>(request.h, std::move(wi));
}



WPBR_END

