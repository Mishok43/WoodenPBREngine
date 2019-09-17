#pragma once

#include "pch.h"
#include "CSpectrum.h"
#include "CBXDF.h"
#include "MicrofaceDistr.h"

WPBR_BEGIN

namespace SBSDFConductorMicroface
{
	float pdf(
		const CReflectDirSamplerMicroface& microface,
		const DVector3f& wo, const DVector3f& wi)
	{
		DVector3f wh = normalize(wo + wi);
		return microface.pdf(wh) / (4 * dot(wo, wh));
	}

	Spectrum f(
		const CReflectDirSamplerMicroface& microface,
		const CSpectrumScale& R,
		const CFresnelConductor& fresnel,
		const DVector3f& wo,
		const DVector3f& wi,
		float& wiPDF
	)
	{
		const DVector3f& wh = normalize(wo + wi);
		float whD = microface.d(wh);

		f_impl(R, fresnel, whD, wh, wo, wi);
	}

	Spectrum f_impl(
		const CSpectrumScale& R,
		const CFresnelConductor& fresnel,
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

		Spectrum fr = fresnel.f(dot(wi, wh));
		return fr * R*(whD / (4 * cosThetaI*cosThetaO));
	}

	Spectrum sample_f(
		const CReflectDirSamplerMicroface& microface,
		const CSpectrumScale& R,
		const CFresnelConductor& fresnel,
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
};
WPBR_END