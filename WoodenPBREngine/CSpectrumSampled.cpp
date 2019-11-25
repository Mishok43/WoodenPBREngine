#include "pch.h"
#include "CSpectrumSampled.h"

WPBR_BEGIN

SampledSpectrum SampledSpectrum::X;
SampledSpectrum SampledSpectrum::Y;
SampledSpectrum SampledSpectrum::Z;
SampledSpectrum SampledSpectrum::rgb2SpectLambda;
SampledSpectrum SampledSpectrum::rgbRefl2SpectWhite;
SampledSpectrum SampledSpectrum::rgbRefl2SpectCyan;
SampledSpectrum SampledSpectrum::rgbRefl2SpectMagenta;
SampledSpectrum SampledSpectrum::rgbRefl2SpectYellow;
SampledSpectrum SampledSpectrum::rgbRefl2SpectRed;
SampledSpectrum SampledSpectrum::rgbRefl2SpectGreen;
SampledSpectrum SampledSpectrum::rgbRefl2SpectBlue;

SampledSpectrum SampledSpectrum::rgbIllum2SpectWhite;
SampledSpectrum SampledSpectrum::rgbIllum2SpectCyan;
SampledSpectrum SampledSpectrum::rgbIllum2SpectMagenta;
SampledSpectrum SampledSpectrum::rgbIllum2SpectYellow;
SampledSpectrum SampledSpectrum::rgbIllum2SpectRed;
SampledSpectrum SampledSpectrum::rgbIllum2SpectGreen;
SampledSpectrum SampledSpectrum::rgbIllum2SpectBlue;


DVector3f toXYZ(const SampledSpectrum& s)
{
	DVector3f xyz;
	xyz.x() = dot(s, SampledSpectrum::X);
	xyz.y() = dot(s, SampledSpectrum::Y);
	xyz.z() = dot(s, SampledSpectrum::Z);
	
	float dl = (sampledLambdaEnd - sampledLambdaStart) / float(nSpectralSamples);
	xyz *= dl;
	return xyz;
}

DVector3f toRGB(const SampledSpectrum& s)
{
	DVector3f xyz = toXYZ(s);
	DVector3f rgb = xyzToRGB(xyz);
	return rgb;
}

WPBR_END