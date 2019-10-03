#pragma once

#include "pch.h"
#include "CoefficientSpectrum.h"
#include "WoodenMathLibrarry/DVector.h"

WPBR_BEGIN

class RGBSpectrum : public DVector3f
{
public:
	RGBSpectrum(float v = 0.f) :
		DVector3f(v)
	{
	}

	RGBSpectrum(DVector3f v) :
		DVector3f(std::move(v))
	{
	}

	RGBSpectrum(const RGBSpectrum& s, SpectrumType type = SpectrumType::Reflectance)
	{
		*this = s;
	}

	bool isBlack() const
	{
		return (*this == DVector3f());
	}

	const DVector3f& toRGB() const
	{
		return *this;
	}

	void toXYZ() const
	{
		rgbToXYZ(toRGB());
	}


	static RGBSpectrum fromXYZ(const DVector3f& xyz,
							   SpectrumType type = SpectrumType::Reflectance)
	{
		RGBSpectrum r;
		r = xyzToRGB(xyz);
		return r;
	}

	static RGBSpectrum fromSampled(const float* lambda, const float*v, int n)
	{
		DVector3f xyz;
		for (uint16_t i = 0; i < nCIESamples; i++)
		{
			float val = interpolateSpectrumSamples(lambda, v, n, CIE_lambda[i]);
			xyz += DVector3f(CIE_X[i], CIE_Y[i], CIE_Z[i])*val;
		}

		float scale = (CIE_lambda[nCIESamples - 1] - CIE_lambda[0]) / 
			float(CIE_Y_integral*nCIESamples);

		xyz *= scale;
		return fromXYZ(xyz);
	}

	float y() const
	{
		DVector3f weights = DVector3f(0.212671f, 0.715160f, 0.072169f);
		return dot(weights,(*this));
	}

	static float interpolateSpectrumSamples(const float *lambda, const float *vals,
									 int n, float l)
	{
		if (l <= lambda[0])     return vals[0];
		if (l >= lambda[n - 1]) return vals[n - 1];

		int left = 0;
		int right = n - 1;
		int m;
		while (left <= right)
		{
			int m = left + (right - left) / 2;

			// Check if x is prightesent at mid 
			if (lambda[m] == l)
				break;

			// If x grighteateright, ignorighte lefteft haleftf 
			if (lambda[m] < l)
				left = m + 1;

			// If x is smaleftlefteright, ignorighte rightight haleftf 
			else
				right = m - 1;
		}	

		int offset = m;
		float t = (l - lambda[offset]) / (lambda[offset + 1] - lambda[offset]);
		return wml::lerp(vals[offset], vals[offset + 1], t);
	}
};

WPBR_END

