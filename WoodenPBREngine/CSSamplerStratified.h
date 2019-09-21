#pragma once

#include "pch.h"
#include "CSSampler.h"
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

class JobSamplerStratifiedGenerateSampels1D: public JobParallazible
{
	static constexpr uint32_t sliceSize = 32;

	virtual uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return std::min(nWorkThreads, (queryComponentsGroup<CSamples1D>().size()+ sliceSize -1)/ sliceSize);
	}

	virtual void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t sliceSize = 
			(queryComponentsGroup<CSamples1D>().size()+nThreads-1)/nThreads;

		ComponentsGroupSlice<CSamples1D> samples =
			queryComponentsGroupSlice<CSamples1D>(Slice(sliceSize*iThread, sliceSize));

		for_each([ecs, this](HEntity hEntity,
				 CSamples1D& samples)
		{			
			std::random_device rd; std::mt19937 gen(rd());
			std::uniform_real_distribution<float> uniform(0.0, 1.0);


			
			for (uint32_t i = 0; i < samples.data.size(); i++)
			{
				float invNSamples = 1.0/ samples.data.size();
				float dt = uniform(gen);
				samples.data[i] = std::min((i + dt)*invNSamples, 1.0);
			}

			std::random_shuffle(samples.data.begin(), samples.data.end());


		}, samples);
	}
};

class JobSamplerStratifiedGenerateSampels2D: public JobParallazible
{
	static constexpr uint32_t sliceSize = 32;

	virtual uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return std::min(nWorkThreads, (queryComponentsGroup<CSamples2D>().size()+ sliceSize -1)/ sliceSize);
	}

	virtual void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t sliceSize = 
			(queryComponentsGroup<CSamples2D>().size()+nThreads-1)/nThreads;

		ComponentsGroupSlice<CSamples2D> samples =
			queryComponentsGroupSlice<CSamples2D>(Slice(sliceSize*iThread, sliceSize));

		for_each([ecs, this](HEntity hEntity,
				 CSamples2D& samples)
		{			
			std::random_device rd; std::mt19937 gen(rd());
			std::uniform_real_distribution<float> uniform(0.0, 1.0);


			for (uint32_t i = 0; i < samples.data.size(); i++)
			{
				for (uint32_t j = 0; j < samples.data.size(); j++)
				{
					float invNSamples = 1.0 / samples.data.size();
					float dtX = uniform(gen);
					float dtY = uniform(gen);
					samples.data[i].x() = std::min((i + dtX)*invNSamples, 1.0);
					samples.data[i].y() = std::min((j + dtY)*invNSamples, 1.0);
				}
			}

			std::random_shuffle(samples.data.begin(), samples.data.end());

		}, samples);
	}
};



WPBR_END;

