#pragma once
#include "pch.h"
#include "CSpectrumSampled.h"
#include "RGBSpectrum.h"

WPBR_BEGIN
using Spectrum = typename SampledSpectrum;

struct CSpectrum: public Spectrum
{
	DECL_MANAGED_DENSE_COMP_DATA(CSpectrum, 16)
};

inline void blackbody(const float* lambda, uint32_t n, float T, float* le)
{
	const float c = 299792458;
	const float h = 6.62606957e-34;
	const float kb = 1.3806488e-23;

	for (uint32_t i = 0; i < n; i++)
	{
		float l = lambda[i] * 1e-9;
		float lambda5 = (l * l) * (l * l) * l;
		le[i] = (2 * h * c * c) /
			(lambda5 * (std::exp((h * c) / (l * kb * T)) - 1));
	}
}

inline void blackbodyNormalized(const float* lambda, uint32_t n, float T, float* le)
{
	blackbody(lambda, n, T, le);
	float lambdaMax = 2.8977721e-3 / T * 1e9;
	float maxL;
	blackbody(&lambdaMax, 1, T, &maxL);
	for (uint32_t i = 0; i < n; i++)
	{
		le[i] /= maxL;
	}
}


WPBR_END