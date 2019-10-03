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
	for (uint32_t i = 0; i < nSpectralSamples / 8; i++)
	{
		xyz[0] += dot(s.c[i], SampledSpectrum::X.c[i]);
		xyz[1] += dot(s.c[i], SampledSpectrum::Y.c[i]);
		xyz[2] += dot(s.c[i], SampledSpectrum::Z.c[i]);
	}

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