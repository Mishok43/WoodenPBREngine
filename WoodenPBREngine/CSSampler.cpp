#include "pch.h"
#include "CSSampler.h"

WPBR_BEGIN


void JobSamplerUpdateCameraSamples::update(WECS* ecs, uint8_t iThread) 
{
	uint32_t sliceSize = (queryComponentsGroup<CCameraSample, CSamples1D, CSamples2D>().size() - nThreads + 1) / getNumThreads();
	ComponentsGroupSlice<CCameraSample, CSamples1D, CSamples2D> tiles =
		queryComponentsGroupSlice<CCameraSample, CSamples1D, CSamples2D>(Slice(sliceSize*iThread, sliceSize));

	for_each([ecs, this](HEntity hEntity,
			 CCameraSample& cameraSample,
			 CSamples1D& samples1D,
			 CSamples2D& samples2D)
	{
	//	cameraSample.pFilm += samples2D.data[samples2D.i++];
	}, tiles);
}

WPBR_END



