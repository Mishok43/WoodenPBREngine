#pragma once

#include "pch.h"
#include "CSCamera.h"
#include "CSFilm.h"
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
	std::vector<DPoint2f, AllocatorAligned<DPoint2f>> data;
	uint32_t i;
};

struct alignas(alignof(DPoint2f)) CCameraSample
{
	DPoint2f pFilm;
};

struct CHEntitySampler: public HEntity
{
	DECL_MANAGED_DENSE_COMP_DATA(CHEntitySampler, 1)
}; DECL_OUT_COMP_DATA(CHEntitySampler)

class JobSamplerUpdateCameraSamples : public JobParallazible
{
	static constexpr  uint32_t sliceSize = 1024;
	virtual void updateNStartThreads(uint8_t nWorkThreads) override
	{
		nThreads = std::min(nWorkThreads, 
			(queryComponentsGroup<CCameraSample, CSamples1D, CSamples2D>().size()+ sliceSize-1)/sliceSize);
	}

	virtual void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t sliceSize = (queryComponentsGroup<CCameraSample, CSamples1D, CSamples2D>().size() - nThreads + 1) / nThreads;
		ComponentsGroupSlice<CCameraSample, CSamples1D, CSamples2D> tiles =
			queryComponentsGroupSlice<CCameraSample, CSamples1D, CSamples2D>(Slice(sliceSize*iThread, sliceSize));

		for_each([ecs, this](HEntity hEntity,
				 CCameraSample& cameraSample,
				 CSamples1D& samples1D,
				 CSamples1D& samples2D)
		{
			cameraSample.pFilm += samples1D.data[samples1D.i++];
		}, tiles);
	}
};


WPBR_END;