#pragma once

#include "pch.h"
#include "CSSampler.h"
#include "WoodenECS/Job.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/DBounds.h"
#include <random>
#include <iostream>

WPBR_BEGIN

struct CSamplerStratified
{
	DECL_MANAGED_DENSE_COMP_DATA(CSamplerStratified, 1)
};

struct CHEntitySamplerStratified : public HEntity
{
	DECL_MANAGED_DENSE_COMP_DATA(CHEntitySamplerStratified, 1)
};

class JobSamplerStratifiedGenerateSampels1D : public JobParallazible
{
	static constexpr uint32_t sliceSize = 32;

	virtual uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return min(nWorkThreads, (queryComponentsGroup<CSamples1D>().size() + sliceSize - 1) / sliceSize);
	}

	virtual void update(WECS* ecs, uint8_t iThread) override;
};

class JobSamplerStratifiedGenerateSampels2D : public JobParallazible
{
	static constexpr uint32_t sliceSize = 32;

	virtual uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return min(nWorkThreads, (queryComponentsGroup<CSamples2D>().size() + sliceSize - 1) / sliceSize);
	}

	virtual void update(WECS* ecs, uint8_t iThread) override;
};



WPBR_END;

