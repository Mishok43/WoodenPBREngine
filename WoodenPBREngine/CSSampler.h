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

	virtual void update(WECS* ecs, uint8_t iThread) override;
};


WPBR_END;