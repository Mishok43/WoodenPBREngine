#pragma once

#include "pch.h"
#include "CSCamera.h"
#include "CFilm.h"
#include "WoodenECS/Job.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/DBounds.h"
#include "CSpectrum.h"
WPBR_BEGIN



struct CSampleIndex
{
	uint16_t val;

	DECL_MANAGED_DENSE_COMP_DATA(CSampleIndex, 16)
}; DECL_OUT_COMP_DATA(CSampleIndex)

struct CSampler
{
	uint32_t nSamplesPerPixel;
	DECL_MANAGED_DENSE_COMP_DATA(CSampler, 1)
}; DECL_OUT_COMP_DATA(CSampler)


struct CSamples1D
{
	std::vector<float, AllocatorAligned<float>> data;
	uint32_t i;
};

struct CSamples2D
{
	std::vector<float, AllocatorAligned<float>> data;
	uint32_t i;
};

struct CCameraSample
{
	DPoint2f pFilm;
	int32_t iSample=0;
};

struct CHEntitySampler: public HEntity
{
	DECL_MANAGED_DENSE_COMP_DATA(CHEntitySampler, 1)
}; DECL_OUT_COMP_DATA(CHEntitySampler)

class JobSamplerCreateCameraSamples : public JobParallazible
{
	virtual void updateNStartThreads(uint8_t nWorkThreads) override
	{
		nThreads = std::min(nWorkThreads, queryComponentsGroup<CCameraSamplesBatch>().size());
	}

	virtual void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t sliceSize = (queryComponentsGroup<CFilmTile, CCameraSamplesBatch, CHEntitySampler>().size() - nThreads + 1) / nThreads;
		ComponentsGroupSlice<CFilmTile, CCameraSamplesBatch, CHEntitySampler> tiles =
			queryComponentsGroupSlice<CFilmTile, CCameraSamplesBatch, CHEntitySampler>(Slice(sliceSize*iThread, sliceSize));

		for_each([ecs, this](HEntity hEntity,
				 const CFilmTile& filmTile,
				 CCameraSamplesBatch& samples,
				 const CHEntitySampler& hEntitySampler)
		{
			CSampler sampler = ecs->getComponent<CSampler>(hEntitySampler);
			wml::for_each(filmTile.pixelBounds, [&](DPoint2i p, DPoint2i ij, uint32_t i)
			{
				samples.pFilm[i] = (DPoint2f)p + samples.samples2D[samples.iSample2D++];
				samples.time[i] = samples.samples1D[samples.iSample1D++];
			});

			samples.iSample++;
			if (samples.iSample == sampler.nSamplesPerPixel)
			{
				samples.bFinishSampling = true;
			}
		}, tiles);
	}
};


WPBR_END;