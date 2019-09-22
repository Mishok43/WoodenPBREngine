#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/DBounds.h"
#include "CSpectrum.h"

WPBR_BEGIN

struct alignas(alignof(DBounds2i)) CFilm
{
	DPoint2i resolution;
	std::vector<uint8_t> rgbOutput;
	std::string outFile;
	uint32_t nProcessedTiles = 0;
};

struct alignas(alignof(DBounds2i)) CFilmTile
{
	DBounds2i pixelBounds;
	std::vector<Spectrum, AllocatorAligned<Spectrum>> pixLiAccumulated;
	std::vector<float, AllocatorAligned<float>> pixFilterWeightAccumulated;
	std::vector<HEntity> samples;
	uint8_t* rgbOutput;
	uint32_t nSamplesPerPixel;

	uint32_t nSamples1DPerCameraSample;
	uint32_t nSamples2DPerCameraSample;
	bool bAccumulated = false;

	uint32_t getPixelIndex(const DPoint2i& p)
	{

		DPoint2i d = p - pixelBounds.pMin;

		return d.x() + d.y()*pixelBounds.diagonal().x();
	}
};

WPBR_END