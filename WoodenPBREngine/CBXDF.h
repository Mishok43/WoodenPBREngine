#pragma once

#include "pch.h"
#include "CSpectrum.h"
#include "WoodenMathLibrarry/Utils.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/DNormal.h"

WPBR_BEGIN
enum BxDFType
{
	BSDF_REFLECTION = 1 << 0,
	BSDF_TRANSMISSION = 1 << 1,
	BSDF_DIFFUSE = 1 << 2,
	BSDF_GLOSSY = 1 << 3,
	BSDF_SPECULAR = 1 << 4,
	BSDF_ALL = BSDF_REFLECTION | BSDF_TRANSMISSION | BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR,
};



float frDielectric(float cosThetaI, float etaI, float etaT)
{
	cosThetaI = wml::clamp(cosThetaI, -1.0, 1.0);
	bool entering = cosThetaI > 0.0f;
	if (!entering)
	{
		std::swap(etaI, etaT);
		cosThetaI = std::abs(cosThetaI);
	}

	float sinThetaI = std::sqrt(std::max(0.0f, 1.0 - cosThetaI * cosThetaI));
	float sinThetaT = etaI / etaT * sinThetaI;
	if (sinThetaT >= 1.0)
	{
		return 1.0;
	}

	float cosThetaT = std::sqrt(std::max(0.0f, 1.0 - sinThetaT * sinThetaT));

	float Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
		((etaT * cosThetaI) + (etaI * cosThetaT));
	float Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
		((etaI * cosThetaI) + (etaT * cosThetaT));
	return (Rparl * Rparl + Rperp * Rperp) / 2;
}




struct CFresnelConductor
{
	Spectrum etaI, etaT, k;

	Spectrum f(float cosThetaI) const
	{
		cosThetaI = wml::clamp(cosThetaI, -1, 1);
		Spectrum eta = etaT / etaI;
		Spectrum etak = k / etaI;

		float cosThetaI2 = cosThetaI * cosThetaI;
		float sinThetaI2 = 1. - cosThetaI2;
		Spectrum eta2 = eta * eta;
		Spectrum etak2 = etak * etak;

		Spectrum t0 = eta2 - etak2 - sinThetaI2;
		Spectrum a2plusb2 = sqrt(t0 * t0 + eta2 * etak2 * 4);
		Spectrum t1 = a2plusb2 + cosThetaI2;
		Spectrum a = sqrt((a2plusb2 + t0)*0.5);
		Spectrum t2 = a * cosThetaI*2.0f;
		Spectrum Rs = (t1 - t2) / (t1 + t2);

		Spectrum t3 = a2plusb2 * cosThetaI2 + sinThetaI2 * sinThetaI2;
		Spectrum t4 = t2 * sinThetaI2;
		Spectrum Rp = Rs * (t3 - t4) / (t3 + t4);

		return (Rp + Rs)*0.5;
	}
};


struct CFresnelDielectric
{
	float etaI, etaT;

	float f(float cosThetaI) const
	{
		cosThetaI = wml::clamp(cosThetaI, -1.0, 1.0);
		float etaItmp = etaI;
		float etaTtmp = etaT;

		bool entering = cosThetaI > 0.0f;
		if (!entering)
		{
			std::swap(etaItmp, etaTtmp);
			cosThetaI = std::abs(cosThetaI);
		}

		float sinThetaI = std::sqrt(std::max(0.0f, 1.0 - cosThetaI * cosThetaI));
		float sinThetaT = etaItmp / etaTtmp * sinThetaI;
		if (sinThetaT >= 1.0)
		{
			return 1.0;
		}

		float cosThetaT = std::sqrt(std::max(0.0f, 1.0 - sinThetaT * sinThetaT));

		float Rparl = ((etaTtmp * cosThetaI) - (etaItmp * cosThetaT)) /
			((etaTtmp * cosThetaI) + (etaItmp * cosThetaT));
		float Rperp = ((etaItmp * cosThetaI) - (etaTtmp * cosThetaT)) /
			((etaItmp * cosThetaI) + (etaTtmp * cosThetaT));
		return (Rparl * Rparl + Rperp * Rperp) / 2;
	}
};

bool refract(const DVector3f& wi, const DNormal3f& n, float eta, DVector3f& wt)
{
	float cosThetaI = dot(n, wi);
	float sin2ThetaI = std::max(0.f, 1.f - cosThetaI * cosThetaI);
	float sin2ThetaT = eta * eta * sin2ThetaI;
	if (sin2ThetaT >= 1)
	{
		return false;
	}

	float cosThetaT = std::sqrt(1 - sin2ThetaT);
	wt = -wi* eta  + DVector3f(n)*(eta * cosThetaI - cosThetaT);
	return true;
}

struct CSpectrumScale : public Spectrum
{
	DECL_MANAGED_DENSE_COMP_DATA(CSpectrumScale, 1)
}; DECL_OUT_COMP_DATA(CSpectrumScale)

struct CBXDF
{
	BxDFType type;
};

struct CBXDFOreanNayar
{
	CBXDFOreanNayar() = default;

	CBXDFOreanNayar(float sigma)
	{
		sigma = radians(sigma);
		float sigma2 = sigma * sigma;
		A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
		B = 0.45f * sigma2 / (sigma2 + 0.09f);
	}

	float A, B;

	Spectrum f(
		const CBXDFOreanNayar& oreanNayar,
		const CSpectrumScale& R,
		const DVector3f& wo,
		const DVector3f& wi) const
	{
		const float& A = oreanNayar.A;
		const float& B = oreanNayar.B;

		float sinThetaI = sinTheta(wi);
		float sinThetaO = sinTheta(wo);
		float maxCos = 0;
		if (sinThetaI > 1e-4 && sinThetaO > 1e-4)
		{
			float sinPhiI = sinPhi(wi), cosPhiI = cosPhi(wi);
			float sinPhiO = sinPhi(wo), cosPhiO = cosPhi(wo);
			float dCos = cosPhiI * cosPhiO + sinPhiI * sinPhiO;
			maxCos = std::max(0.0f, dCos);
		}

		float sinAlpha, tanBeta;
		if (absCosTheta(wi) > absCosTheta(wo))
		{
			sinAlpha = sinThetaO;
			tanBeta = sinThetaI / absCosTheta(wi);
		}
		else
		{
			sinAlpha = sinThetaI;
			tanBeta = sinThetaO / absCosTheta(wo);
		}

		return R * 1.0 / PI * (A + B * maxCos*sinAlpha*tanBeta);
	}
};

struct CBXDFLambertian
{
	Spectrum f(
		const CBXDFOreanNayar& lambertian,
		const CSpectrumScale& R,
		const DVector3f& wo,
		const DVector3f& wi) const
	{
		return R * 1.0 / PI;
	}
};



Spectrum sample_f(
	const CSpectrumScale& sScale,
	const CFresnelConductor& fresnel,
	const DVector3f& wo,
	DVector3f& wi,
	const DPoint2f& sample,
	float& pdf, BxDFType* sampledType)
{
	wi = DVector3f(-wo.x(), -wo.y(), wo.z());
	pdf = 1.0f;	

	float cTheta = cosTheta(wo);
	return fresnel.f(cTheta)*sScale*1.0 / abs(cTheta);
}

Spectrum sample_f(
	const CSpectrumScale& sScale,
	const CFresnelDielectric& fresnel,
	const DVector3f& wo,
	DVector3f& wi,
	const DPoint2f& sample,
	float& pdf, BxDFType* sampledType,
	TransportMode mode)
{
	float cTheta = cosTheta(wo);
	cTheta = wml::clamp(cTheta, -1.0, 1.0);
	float etaItmp = fresnel.etaI;
	float etaTtmp = fresnel.etaT;
	bool entering = cTheta > 0;
	etaItmp = (entering) ? fresnel.etaI : fresnel.etaT;
	etaTtmp = (entering) ? fresnel.etaT : fresnel.etaI;

	if (!refract(wo, faceForward(DNormal3f(0.0, 0.0, 1.0), wo), etaItmp / etaTtmp, wi))
	{
		return 0;
	}

	pdf = 1.0f;	

	Spectrum ft = sScale*(Spectrum(1.0)- fresnel.f(cosTheta(wi)))*1.0;
	if (mode == TransportMode::Radiance)
	{
		ft = ft*(etaItmp*etaItmp) / (etaTtmp*etaTtmp);
	}
	return ft / abs(cosTheta(wi));
}



WPBR_END