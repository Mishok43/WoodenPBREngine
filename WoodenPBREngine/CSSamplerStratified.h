#pragma once

#include "pch.h"
#include "CSSampler.h"
#include "CFilm.h"
#include "WoodenECS/Job.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/DBounds.h"
#include <random>
#include <algorithm>

WPBR_BEGIN

struct CSamplerStratified
{
	DECL_MANAGED_DENSE_COMP_DATA(CSamplerStratified, 1)
}; DECL_OUT_COMP_DATA(CSamplerStratified)

struct CHEntitySamplerStratified: public HEntity
{
	DECL_MANAGED_DENSE_COMP_DATA(CHEntitySamplerStratified, 1)
}; DECL_OUT_COMP_DATA(CHEntitySamplerStratified)

class JobSamplerStratifiedGenerateSampels: public JobParallazible
{
	virtual void updateNStartThreads(uint8_t nWorkThreads) override
	{
		nThreads = std::min(nWorkThreads, queryComponentsGroup<CCameraSamplesBatch>().size());
	}

	virtual void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t sliceSize = (queryComponentsGroup<CCameraSamplesBatch, CHEntitySamplerStratified>().size() - nThreads + 1) / nThreads;
		ComponentsGroupSlice<CCameraSamplesBatch, CHEntitySamplerStratified> tiles =
			queryComponentsGroupSlice<CCameraSamplesBatch, CHEntitySamplerStratified>(Slice(sliceSize*iThread, sliceSize));

		for_each([ecs, this](HEntity hEntity,
				 CCameraSamplesBatch& samples,
				 const CHEntitySamplerStratified& hEntitySampler)
		{
			CHEntitySamplerStratified sampler = ecs->getComponent<CHEntitySamplerStratified>(hEntitySampler);
			
			std::random_device rd; std::mt19937 gen(rd());
			std::uniform_real_distribution<float> uniform(0.0, 1.0);
			
			for (uint32_t i = 0; i < samples.samples1D.size(); i++)
			{
				float invNSamples = 1.0/samples.samples1D.size();
				float dt = uniform(gen);
				samples.samples1D[i] = std::min((i + dt)*invNSamples, 1.0);
			}

			std::random_shuffle(samples.samples1D.begin(), samples.samples1D.end());

			for (uint32_t i = 0; i < samples.samples2D.size(); i++)
			{
				for (uint32_t j = 0; j < samples.samples2D.size(); j++)
				{
					float invNSamples = 1.0 / samples.samples2D.size();
					float dtX = uniform(gen);
					float dtY = uniform(gen);
					samples.samples2D[i].x() = std::min((i + dtX)*invNSamples, 1.0);
					samples.samples2D[i].y() = std::min((j + dtY)*invNSamples, 1.0);
				}
			}

			std::random_shuffle(samples.samples2D.begin(), samples.samples2D.end());

		}, tiles);
	}
};



WPBR_END;

