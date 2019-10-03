#include "pch.h"
#include "CSpectrum.h"
WPBR_BEGIN
DECL_OUT_COMP_DATA(CSpectrum)


inline Spectrum lerp(const Spectrum& s0, const Spectrum& s1, float t)
{
	return s0 * (1 - t) + s1 * t;
}

WPBR_END

