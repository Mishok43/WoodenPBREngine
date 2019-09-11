#pragma once
#include "pch.h"
#include "CSpectrumSampled.h"
#include "RGBSpectrum.h"

WPBR_BEGIN
using Spectrum = typename CoefficientSpectrum<4>;

Spectrum lerp(const Spectrum& s0, const Spectrum& s1, float t)
{
	return s0 * (1 - t) + s1 * t;
}

WPBR_END