#pragma once

#include "pch.h"
#include <array>
#include <algorithm>
#include "WoodenMathLibrarry/DVector.h"
#include "WoodenMathLibrarry/DMatrix.h"
#include "WoodenMathLibrarry/Utils.h"
#include "CoefficientSpectrum.h"
#include "RGBSpectrum.h"

WPBR_BEGIN

class SampledSpectrum : public CoefficientSpectrum<nSpectralSamples/8>
{
public:
	explicit SampledSpectrum(float v = 0.0f) :
		CoefficientSpectrum(v)
	{}

	SampledSpectrum(const CoefficientSpectrum& r):
		CoefficientSpectrum(r)
	{}

	SampledSpectrum(const RGBSpectrum& r, SpectrumType t =  SpectrumType::Reflectance)
	{
		*this = SampledSpectrum::fromRGB(r, t);
	}



	static float averageSpectrumSamples(const float* lambda, const float *values, int n, 
										float waveLength0, float waveLength1)
	{
		if (waveLength1 <= lambda[0])
		{
			return values[0];
		}
		if (waveLength0 >= lambda[n - 1])
		{
			return values[n - 1];
		}
		if (n == 1)
		{
			return values[0];
		}


		float sumSamples = 0;
		if (waveLength0 < lambda[0])
			sumSamples += values[0] * (lambda[0] - waveLength0);
		if (waveLength1 > lambda[n - 1])
			sumSamples += values[n - 1] * (waveLength1 - lambda[n - 1]);

		uint16_t j = 0;
		while (waveLength0 > lambda[j + 1])
		{
			j++;
		}

		auto interp = [lambda, values](float w, int i)
		{
			return wml::lerp(values[i], values[i + 1], (w - lambda[i]) / (lambda[i + 1] - lambda[i]));
		};


		for (; j + 1 < n && waveLength1 >= lambda[j]; j++)
		{
			float segWaveLength0 = std::max(waveLength0, lambda[i]);
			float segWaveLength1 = std::min(waveLength1, lambda[i + 1]);
			sumSamples += (interp(segWaveLength0, i) + interp(segWaveLength1, i)) *
				(segWaveLength1 - segWaveLength0)*0.5;
		}
		return sumSamples / (waveLength1 - waveLength0);
	}

	static SampledSpectrum fromSampled(const float* lambda, const float *values, int n)
	{
		SampledSpectrum r;

		float waveLength0 = sampledLambdaStart;
		float step = (sampledLambdaEnd - sampledLambdaStart) / (nSpectralSamples-1);
		for (uint32_t i = 0; i < nSpectralSamples; i++)
		{
			float waveLength1 = waveLength0 + step;
			r[i] = averageSpectrumSamples(lambda, values, n, waveLength0, waveLength1);
		}
	}

	static SampledSpectrum fromRGB(const DVector3f& rgb,
								   SpectrumType type)
	{
		SampledSpectrum s;
		if (type == SpectrumType::Reflectance)
		{
			bool bRedIsMin = rgb[0] <= rgb[1] && rgb[0] <= rgb[2];
			bool bGreenIsMin = rgb[1] <= rgb[0] && rgb[1] <= rgb[2];

			if (bRedIsMin)
			{
				s += rgbRefl2SpectWhite * rgb[0];
				if (rgb[1] <= rgb[2])
				{
					// total = white*smallRed + (middleGreen-smallRed)*cyan + (bigBlue-middleGreen)*blue
					s += rgbRefl2SpectCyan * (rgb[1] - rgb[0]); // cyan -> gree+blue
					s += rgbRefl2SpectBlue * (rgb[2] - rgb[1]);
				}
				else
				{
					s += rgbRefl2SpectCyan * (rgb[2] - rgb[0]);
					s += rgbRefl2SpectGreen * (rgb[1] - rgb[2]);
				}
			}
			else if (bGreenIsMin)
			{
				s += rgbRefl2SpectWhite * rgb[1];
				if (rgb[0] <= rgb[2])
				{
					s += rgbRefl2SpectMagenta * (rgb[0] - rgb[1]);
					s += rgbRefl2SpectBlue * (rgb[2] - rgb[0]);
				}
				else
				{
					s += rgbRefl2SpectMagenta * (rgb[2] - rgb[1]);
					s += rgbRefl2SpectRed * (rgb[0] - rgb[2]);
				}
			}
			else
			{
				s += rgbRefl2SpectWhite * rgb[2];
				if (rgb[0] <= rgb[1])
				{
					s += rgbRefl2SpectYellow * (rgb[0] - rgb[2]);
					s += rgbRefl2SpectGreen * (rgb[1] - rgb[0]);
				}
				else
				{
					s += rgbRefl2SpectYellow * (rgb[1] - rgb[2]);
					s += rgbRefl2SpectRed * (rgb[0] - rgb[1]);
				}
			}
		}
		else
		{
			bool bRedIsMin = rgb[0] <= rgb[1] && rgb[0] <= rgb[2];
			bool bGreenIsMin = rgb[1] <= rgb[0] && rgb[1] <= rgb[2];

			if (bRedIsMin)
			{
				s += rgbIllum2SpectWhite * rgb[0];
				if (rgb[1] <= rgb[2])
				{
					// total = white*smallRed + (middleGreen-smallRed)*cyan + (bigBlue-middleGreen)*blue
					s += rgbIllum2SpectCyan * (rgb[1] - rgb[0]); // cyan -> gree+blue
					s += rgbIllum2SpectBlue * (rgb[2] - rgb[1]);
				}
				else
				{
					s += rgbIllum2SpectCyan * (rgb[2] - rgb[0]);
					s += rgbIllum2SpectGreen * (rgb[1] - rgb[2]);
				}
			}
			else if (bGreenIsMin)
			{
				s += rgbIllum2SpectWhite * rgb[1];
				if (rgb[0] <= rgb[2])
				{
					s += rgbIllum2SpectMagenta * (rgb[0] - rgb[1]);
					s += rgbIllum2SpectBlue * (rgb[2] - rgb[0]);
				}
				else
				{
					s += rgbIllum2SpectMagenta * (rgb[2] - rgb[1]);
					s += rgbIllum2SpectRed * (rgb[0] - rgb[2]);
				}
			}
			else
			{
				s += rgbIllum2SpectWhite * rgb[2];
				if (rgb[0] <= rgb[1])
				{
					s += rgbIllum2SpectYellow * (rgb[0] - rgb[2]);
					s += rgbIllum2SpectGreen * (rgb[1] - rgb[0]);
				}
				else
				{
					s += rgbIllum2SpectYellow * (rgb[1] - rgb[2]);
					s += rgbIllum2SpectRed * (rgb[0] - rgb[1]);
				}
			}
		}
		return s;
	}

	static SampledSpectrum fromXYZ(const DVector3f& xyz,
								   SpectrumType type = SpectrumType::Reflectance)
	{
		DVector3f rgb = xyzToRGB(xyz);
		return fromRGB(rgb, type);
	}
	
	static SampledSpectrum X, Y, Z;
	static SampledSpectrum rgb2SpectLambda;
	static SampledSpectrum rgbRefl2SpectWhite;
	static SampledSpectrum rgbRefl2SpectCyan;
	static SampledSpectrum rgbRefl2SpectMagenta;
	static SampledSpectrum rgbRefl2SpectYellow;
	static SampledSpectrum rgbRefl2SpectRed;
	static SampledSpectrum rgbRefl2SpectGreen;
	static SampledSpectrum rgbRefl2SpectBlue;

	static SampledSpectrum rgbIllum2SpectWhite;
	static SampledSpectrum rgbIllum2SpectCyan;
	static SampledSpectrum rgbIllum2SpectMagenta;
	static SampledSpectrum rgbIllum2SpectYellow;
	static SampledSpectrum rgbIllum2SpectRed;
	static SampledSpectrum rgbIllum2SpectGreen;
	static SampledSpectrum rgbIllum2SpectBlue;

	static void init()
	{
		for (uint16_t i = 0; i < nSpectralSamples; i++)
		{
			float wl0 = wml::lerp(sampledLambdaStart, sampledLambdaEnd, float(i) / float(nSpectralSamples));
			float wl1 = wml::lerp(sampledLambdaStart, sampledLambdaEnd, float(i+1) / float(nSpectralSamples));
			X[i] = averageSpectrumSamples(CIE_lambda, CIE_X, nCIESamples, wl0, wl1);
			Y[i] = averageSpectrumSamples(CIE_lambda, CIE_Y, nCIESamples, wl0, wl1);
			Z[i] = averageSpectrumSamples(CIE_lambda, CIE_Z, nCIESamples, wl0, wl1);
		}

		for (int i = 0; i < nSpectralSamples; ++i)
		{
			float wl0 = wml::lerp(sampledLambdaStart, sampledLambdaEnd, float(i) / float(nSpectralSamples));
			float wl1 = wml::lerp(sampledLambdaStart, sampledLambdaEnd, float(i + 1) / float(nSpectralSamples));
			rgbRefl2SpectWhite[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectWhite,
															 nRGB2SpectSamples, wl0, wl1);
			rgbRefl2SpectCyan[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectCyan,
															nRGB2SpectSamples, wl0, wl1);
			rgbRefl2SpectMagenta[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectMagenta,
															   nRGB2SpectSamples, wl0, wl1);
			rgbRefl2SpectYellow[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectYellow,
															  nRGB2SpectSamples, wl0, wl1);
			rgbRefl2SpectRed[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectRed,
														   nRGB2SpectSamples, wl0, wl1);
			rgbRefl2SpectGreen[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectGreen,
															 nRGB2SpectSamples, wl0, wl1);
			rgbRefl2SpectBlue[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectBlue,
															nRGB2SpectSamples, wl0, wl1);

			rgbIllum2SpectWhite[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectWhite,
															  nRGB2SpectSamples, wl0, wl1);
			rgbIllum2SpectCyan[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectCyan,
															 nRGB2SpectSamples, wl0, wl1);
			rgbIllum2SpectMagenta[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectMagenta,
																nRGB2SpectSamples, wl0, wl1);
			rgbIllum2SpectYellow[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectYellow,
															   nRGB2SpectSamples, wl0, wl1);
			rgbIllum2SpectRed[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectRed,
															nRGB2SpectSamples, wl0, wl1);
			rgbIllum2SpectGreen[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectGreen,
															  nRGB2SpectSamples, wl0, wl1);
			rgbIllum2SpectBlue[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectBlue,
															 nRGB2SpectSamples, wl0, wl1);
		}
	}

	static DVector3f toXYZ(const SampledSpectrum& s) 
	{
		DVector3f xyz;
		for (uint32_t i = 0; i < nSpectralSamples/8; i++)
		{
			xyz[0] += dot(s.c[i],X.c[i]);
			xyz[1] += dot(s.c[i],Y.c[i]);
			xyz[2] += dot(s.c[i],Z.c[i]);
		}

		float dl = (sampledLambdaEnd - sampledLambdaStart) / float(nSpectralSamples);
		xyz *= dl;
		return xyz;
	}


	float y() const
	{
		float yy = 0.0f;
		for (uint32_t i = 0; i < nSpectralSamples / 8; i++)
		{
			yy += dot(c[i], Y.c[i]);
		}
		float dl = (sampledLambdaEnd - sampledLambdaStart) / float(nSpectralSamples);
		return yy * dl;
	}
};


DVector3f toRGB(const SampledSpectrum& s)
{
	DVector3f xyz = toXYZ(s);
	DVector3f rgb = xyzToRGB(xyz);
	return rgb;
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

template<uint8_t nSamplesGroups>
CoefficientSpectrum<nSamplesGroups> sqrt(const CoefficientSpectrum<nSamplesGroups>& s)
{
	CoefficientSpectrum ret;
	for (uint32_t i = 0; i < nSamplesGroups; i++)
	{
		ret.c[i] = sqrt(s.c[i]);
	}
}

WPBR_END

