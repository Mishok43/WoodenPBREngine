#pragma once

//#define USE_SLOW_SAMPLED_SPECTRUM

#ifndef USE_SLOW_SAMPLED_SPECTRUM
using CSpectrum = typename class CSpectrumRGB;
#else
using CSpectrum = typename class CSpectrumSampled;
#endif

