#include "pch.h"
#include "CSpectrum.h"
WPBR_BEGIN
DECL_OUT_COMP_DATA(CSpectrum)


inline Spectrum lerp(const Spectrum& s0, const Spectrum& s1, float t)
{
	return s0 * (1 - t) + s1 * t;
}

inline Spectrum exp(const Spectrum& s0)
{
	Spectrum s = s0;
	for (uint32_t i = 0; i < s0.size(); i++)
	{
		s[i] = std::exp(s[i]);
	}
	return s;
}

WPBR_END

