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


WPBR_END