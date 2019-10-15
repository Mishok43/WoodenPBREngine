#include "pch.h"
#include "CSSampler.h"

WPBR_BEGIN


void JobSamplerUpdateCameraSamples::update(WECS* ecs, uint8_t iThread) 
{
	uint32_t sliceSize = (queryComponentsGroup<CCameraRasterPoint, CSamples1D, CSamples2D>().size() - nThreads + 1) / getNumThreads();
	ComponentsGroupSlice<CCameraRasterPoint, CSamples1D, CSamples2D> tiles =
		queryComponentsGroupSlice<CCameraRasterPoint, CSamples1D, CSamples2D>(Slice(sliceSize*iThread, sliceSize));

	for_each([ecs, this](HEntity hEntity,
			 CCameraRasterPoint& cameraRaster,
			 CSamples1D& samples1D,
			 CSamples2D& samples2D)
	{
		CCameraSample s;
		s.pFilm = cameraRaster.p + samples2D.next();
		ecs->addComponent<CCameraSample>(hEntity, std::move(s));
		ecs->addComponent<CSpectrum>(hEntity);
	}, tiles);
}

WPBR_END



