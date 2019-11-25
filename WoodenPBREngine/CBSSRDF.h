#pragma once
#include "pch.h"
#include "MEngine.h"
#include "CSpectrum.h"
#include "CSufraceInteraction.h"
#include "CBXDF.h"


WPBR_BEGIN

struct CBSSRDF
{
	HEntity hSIOut;
	float eta;
};



template<typename BSSRDFT>
class SSeparableBSSRDF
{
public:
	static Spectrum evaluate(const CSurfaceInteraction& siOut, float eta, const CSurfaceInteraction& siIn, const DVector3f& wi)
	{
		CFresnelDielectric fresnel;
		fresnel.etaI = 1.0f;
		fresnel.etaT = eta;

		float reflectedOut = 1.0f - fresnel(dot(siOut.wo, siOut.shading.n));
		return sp(siOut, siIn)*(reflectedOut * sw(eta, wi));
	};

	static float fresnelMoment1(float eta)
	{
		float eta2 = eta * eta, eta3 = eta2 * eta, eta4 = eta3 * eta,
			eta5 = eta4 * eta;
		if (eta < 1)
			return 0.45966f - 1.73965f * eta + 3.37668f * eta2 - 3.904945 * eta3 +
			2.49277f * eta4 - 0.68441f * eta5;
		else
			return -4.61686f + 11.1136f * eta - 10.4646f * eta2 + 5.11455f * eta3 -
			1.27198f * eta4 + 0.12746f * eta5;
	}

	static float sw(float eta, const DVector3f& wi)
	{
		float c = 1.0f - 2 * fresnelMoment1(1.0 / eta);

		CFresnelDielectric fresnel;
		fresnel.etaI = 1.0f;
		fresnel.etaT = eta;
		return (1.0 - fresnel(cosTheta(wi))) / (c*PI);
	}

	static Spectrum sp(const CSurfaceInteraction& siOut, const CSurfaceInteraction& siIn)
	{
		return BSSRDFT::sr((siOut.p - siIn.p).length());
	}
};


struct CTabulatedBSSRDF
{
	Spectrum tr;
	Spectrum rho;

	DECL_MANAGED_DENSE_COMP_DATA(CTabulatedBSSRDF, 1)
};

struct CBSSRDFTableEntity
{
	HEntity h;

	DECL_MANAGED_DENSE_COMP_DATA(CBSSRDFTableEntity, 1)
};

struct CBSSRDFTable
{
	const int nRhoSamples, nRadiusSamples;
	std::vector<float> rhoSamples, radiusSamples;
	std::vector<float> profile;
	std::vector<float> rhoEff;
	std::vector<float> profileCDF;


	float eval(uint32_t iRho, uint32_t iRadius) const{ return profile[iRho*nRadiusSamples + iRadius]; }

	DECL_MANAGED_DENSE_COMP_DATA(CBSSRDFTable, 1)
};


class STabulatedBSSRDF
{
public:
	static HEntity create(const Spectrum& absorb, const Spectrum& scatter, float eta, HEntity hTable);

	static Spectrum sr(const CTabulatedBSSRDF& bssrdf, const CBSSRDFTable& table, float r);
	
};

WPBR_END