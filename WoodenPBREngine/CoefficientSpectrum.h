#pragma once
#include "pch.h"

WPBR_BEGIN

static const int nCIESamples = 471;
extern const float CIE_X[nCIESamples];
extern const float CIE_Y[nCIESamples];
extern const float CIE_Z[nCIESamples];
extern const float CIE_lambda[nCIESamples];
extern const float CIE_Y_integral[nCIESamples];

static const int nRGB2SpectSamples = 32;
extern const float RGB2SpectLambda[nRGB2SpectSamples];
extern const float RGBRefl2SpectWhite[nRGB2SpectSamples];
extern const float RGBRefl2SpectCyan[nRGB2SpectSamples];
extern const float RGBRefl2SpectMagenta[nRGB2SpectSamples];
extern const float RGBRefl2SpectYellow[nRGB2SpectSamples];
extern const float RGBRefl2SpectRed[nRGB2SpectSamples];
extern const float RGBRefl2SpectGreen[nRGB2SpectSamples];
extern const float RGBRefl2SpectBlue[nRGB2SpectSamples];

extern const float RGBIllum2SpectWhite[nRGB2SpectSamples];
extern const float RGBIllum2SpectCyan[nRGB2SpectSamples];
extern const float RGBIllum2SpectMagenta[nRGB2SpectSamples];
extern const float RGBIllum2SpectYellow[nRGB2SpectSamples];
extern const float RGBIllum2SpectRed[nRGB2SpectSamples];
extern const float RGBIllum2SpectGreen[nRGB2SpectSamples];
extern const float RGBIllum2SpectBlue[nRGB2SpectSamples];


static const uint32_t sampledLambdaStart = 400;
static const uint32_t sampledLambdaEnd = 700;
static const uint32_t nSpectralSamples = 64;

enum class SpectrumType
{
	Reflectance,
	Illuminant
};

DVector3f xyzToRGB(const DVector3f& xyz)
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

DVector3f rgbToXYZ(const DVector3f& rgb)
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
class CoefficientSpectrum
{
public:
	std::array<DVector8f, nSamplesGroups> c;

	CoefficientSpectrum(float v = 0.0f)
	{
		for (uint32_t i = 0; i < nSamplesGroups; i++)
		{
			c[i] = DVector8f(i);
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

	CoefficientSpectrum& operator+(const CoefficientSpectrum& s2) const
	{
		CoefficientSpectrum s1 = *this;
		for (uint32_t i = 0; i < nSamplesGroups; i++)
		{
			s1.c[i] += s2.c[i];
		}
		return s1;
	}

	CoefficientSpectrum& operator-(const CoefficientSpectrum& s2) const
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
CoefficientSpectrum<nSamples> sqrt(const CoefficientSpectrum<nSamples>& s)
{
	CoefficientSpectrum r;
	for (uint8_t i = 0; i < nSamples; i++)
	{
		r.c[i] = sqrt(s.c[i]);
	}
	return s;
}


template<uint8_t nSamplesGroups>
CoefficientSpectrum<nSamplesGroups> clamp(const CoefficientSpectrum<nSamplesGroups>& s, float low = 0, float high = std::numeric_limits<float>::infinity())
{
	CoefficientSpectrum ret = s;
	for (uint32_t i = 0; i < nSamplesGroups; i++)
	{
		ret.c[i] = clamp(s.c[i], low, high);
	}
	return ret;
}

WPBR_END