#pragma once
#include "pch.h"
#include "SpectrumData.h"
#include "WoodenMathLibrarry/DMatrix.h"

WPBR_BEGIN

enum class SpectrumType
{
	Reflectance,
	Illuminant
};

inline DVector3f xyzToRGB(const DVector3f& xyz)
{
	DMatrixf transform = DMatrixf(
		3.240479f, -0.969256f, 0.041556f, 0,
		-1.537150f, 1.875991f, -0.204043f, 0,
		-0.498535f, 0.041556f, 1.057311f, 0,
		0, 0, 0, 0
	);
	DVector3f rgb = xyz * transform;
	return rgb;
}

inline DVector3f rgbToXYZ(const DVector3f& rgb)
{
	DMatrixf transform = DMatrixf(
		0.412453f, 0.212671f, 0.019334f, 0,
		0.357580f, 0.715160f, 0.119193f, 0,
		0.180423f, 0.072169f, 0.950227f, 0,
		0, 0, 0, 0
	);
	DVector3f xyz = rgb * transform;
	return xyz;
}



template<uint8_t nSamplesGroups>
class alignas(alignof(DVector8f)) CoefficientSpectrum
{
public:
	std::array<DVector8f, nSamplesGroups> c;

	CoefficientSpectrum(float v = 0.0f)
	{
		for (uint32_t i = 0; i < nSamplesGroups; i++)
		{
			c[i] = DVector8f(v);
		}
	}

	CoefficientSpectrum &operator+=(const CoefficientSpectrum& s2)
	{
		for (uint32_t i = 0; i < nSamplesGroups; i++)
		{
			c[i] += s2.c[i];
		}
		return *this;
	}

	CoefficientSpectrum &operator-=(const CoefficientSpectrum& s2)
	{
		for (uint32_t i = 0; i < nSamplesGroups; i++)
		{
			c[i] -= s2.c[i];
		}
		return *this;
	}


	CoefficientSpectrum operator+(const CoefficientSpectrum& s2) const
	{
		CoefficientSpectrum s1 = *this;
		for (uint32_t i = 0; i < nSamplesGroups; i++)
		{
			s1.c[i] += s2.c[i];
		}
		return s1;
	}

	CoefficientSpectrum operator-(const CoefficientSpectrum& s2) const
	{
		CoefficientSpectrum s1 = *this;
		for (uint32_t i = 0; i < nSamplesGroups; i++)
		{
			s1.c[i] -= s2.c[i];
		}
		return s1;
	}


	CoefficientSpectrum operator*(float f) const
	{
		CoefficientSpectrum s = *this;
		for (uint32_t i = 0; i < nSamplesGroups; i++)
		{
			s.c[i] *= f;
		}
		return s;
	}


	CoefficientSpectrum operator/(CoefficientSpectrum s2) const
	{
		CoefficientSpectrum s = *this;
		for (uint32_t i = 0; i < nSamplesGroups; i++)
		{
			s.c[i] /= s2.c[i];
		}
		return s;
	}

	CoefficientSpectrum operator*(CoefficientSpectrum s2) const
	{
		CoefficientSpectrum s = *this;
		for (uint32_t i = 0; i < nSamplesGroups; i++)
		{
			s.c[i] *= s2.c[i];
		}
		return s;
	}


	bool isBlack() const
	{
		for (uint32_t i = 0; i < nSamplesGroups; i++)
		{
			if (c[i] != 0.0)
			{
				return false;
			}
		}
		return true;
	}

	float& operator[](uint32_t i)
	{
		return c[i / 8][i % 8];
	}
};


template<uint8_t nSamples>
inline CoefficientSpectrum<nSamples> sqrt(const CoefficientSpectrum<nSamples>& s)
{
	CoefficientSpectrum<nSamples> r;
	for (uint8_t i = 0; i < nSamples; i++)
	{
		r.c[i] = sqrt(s.c[i]);
	}
	return r;
}


template<uint8_t nSamplesGroups>
inline CoefficientSpectrum<nSamplesGroups> clamp(const CoefficientSpectrum<nSamplesGroups>& s, float low = 0, float high = std::numeric_limits<float>::infinity())
{
	CoefficientSpectrum ret = s;
	for (uint32_t i = 0; i < nSamplesGroups; i++)
	{
		ret.c[i] = clamp(s.c[i], low, high);
	}
	return ret;
}



WPBR_END