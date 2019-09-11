#pragma once
#include "pch.h"
#include "WoodenMathLibrarry/DBounds.h"
#include "WoodenECS/Job.h"
#include "CSpectrum.h"
#include "CSCamera.h"

WPBR_BEGIN


struct alignas(alignof(DBounds2i)) CFilmTile
{
	DBounds2i pixelBounds;
	std::vector<Spectrum, AllocatorAligned<Spectrum>> pixLiAccumulated;
	std::vector<float, AllocatorAligned<float>> pixFilterWeightAccumulated;
	float* rgbOutput;
	bool bAccumulated = false;

	uint32_t getPixelIndex(const DPoint2i& p)
	{
		assert(p >= pixelBounds.pMin);
		assert(p <= pixelBounds.pMax);

		DPoint2i d = p - pixelBounds.pMin;

		return d.x() + d.y()*pixelBounds.diagonal().x();
	}
};

struct MitchellFilter
{
public:
	MitchellFilter(float r, float b, float c):
		radius(r), B(b), C(c){ }

	float evaluate(const DPoint2f& p) const
	{
		return evaluate(p.x() / radius)*evaluate(p.y() / radius);
	}

	float evaluate(float x) const
	{
		x = std::abs(2 * x);
		if (x > 1)
			return ((-B - 6 * C) * x*x*x + (6 * B + 30 * C) * x*x +
			(-12 * B - 48 * C) * x + (8 * B + 24 * C)) * (1.f / 6.f);
		else
			return ((12 - 9 * B - 6 * C) * x*x*x +
			(-18 + 12 * B + 6 * C) * x*x +
					(6 - 2 * B)) * (1.f / 6.f);
	}

	float radius,  B,  C;
};


class JobAccumalateLIFromSamples : public JobParallazible
{
	void updateNStartThreads(uint8_t nWorkThreads) override
	{
		nThreads = nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t sliceSize = (queryComponentsGroup<CFilmTile, CCameraSamplesBatch>().size() - nThreads + 1) / nThreads;
		ComponentsGroupSlice<CFilmTile, CCameraSamplesBatch> samples =
		queryComponentsGroupSlice<CFilmTile, CCameraSamplesBatch>(Slice(sliceSize*iThread, sliceSize));

		for_each([&](HEntity, CFilmTile& tile, CCameraSamplesBatch& samples)
		{
			for (uint32_t i = 0; i < samples.pFilm.size(); i++)
			{
				const static MitchellFilter filter(4.0, 2, -0.5);

				HEntity rayEntity = samples.raysEntities[i];

				const DPoint2f& samplePos = samples.pFilm[i];


				const CSpectrum& li = ecs->getComponent<CSpectrum>(rayEntity);
				
				DPoint2f pDiscrete = samplePos - DVector2f(0.5, 0.5);
				DPoint2i pMin = (DPoint2i)ceil(pDiscrete - filter.radius);
				DPoint2i pMax = (DPoint2i)floor(pDiscrete + filter.radius);
				pMin = maxv(pMin, tile.pixelBounds.pMin);
				pMax = minv(pMax, tile.pixelBounds.pMax);

				for (uint16_t y = pMin.y(); y <= pMax.y(); y++)
				{
					for (uint16_t x = pMin.x(); x <= pMax.x(); x++)
					{
						float weight = filter.evaluate(DPoint2f(x, y) - samplePos);
						int32_t iPixel = tile.getPixelIndex(DPoint2i(x, y));
						tile.pixLiAccumulated[iPixel] += weight * li;
						tile.pixFilterWeightAccumulated[iPixel] += weight;
					}
				}	
			}
		}, samples);
	}
};

class JobOutputFilmTitles : public JobParallazible
{
	void updateNStartThreads(uint8_t nWorkThreads) override
	{
		nThreads = nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t sliceSize = (queryComponentsGroup<CFilmTile>().size() - nThreads + 1) / nThreads;
		ComponentsGroupSlice<CFilmTile> samples =
		queryComponentsGroupSlice<CFilmTile>(Slice(sliceSize*iThread, sliceSize));

		for_each([&](HEntity, CFilmTile& tile)
		{
			wml::for_each(tile.pixelBounds, 
			[&](const DPoint2i& pixel, const DPoint2i& ij, uint32_t i)
			{
				DVector3f rgb= tile.pixLiAccumulated[i].toRGB() / tile.pixFilterWeightAccumulated[i];
				tile.rgbOutput[i * 3 + 0] = rgb.x();
				tile.rgbOutput[i * 3 + 1] = rgb.y();
				tile.rgbOutput[i * 3 + 2] = rgb.z();
			});
		}, samples);
	}
};

WPBR_END

