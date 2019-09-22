#pragma once

#include "pch.h"
#include "CSCamera.h"
#include "WoodenECS/Job.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/DBounds.h"
#include "CSpectrum.h"
#include "CSampler.h"
WPBR_BEGIN



class JobSamplerUpdateCameraSamples : public JobParallazible
{
	static constexpr  uint32_t sliceSize = 1024;
	virtual uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return min(nWorkThreads, 
			(queryComponentsGroup<CCameraSample, CSamples1D, CSamples2D>().size()+ sliceSize-1)/sliceSize);
	}

	virtual void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t sliceSize = (queryComponentsGroup<CCameraSample, CSamples1D, CSamples2D>().size() - nThreads + 1) /getNumThreads();
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