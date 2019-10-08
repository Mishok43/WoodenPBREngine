#include "pch.h"
#include "CSSamplerStratified.h"

WPBR_BEGIN
DECL_OUT_COMP_DATA(CSamplerStratified)
DECL_OUT_COMP_DATA(CHEntitySamplerStratified)

void JobSamplerStratifiedGenerateSampels1D::update(WECS* ecs, uint8_t iThread)
{
	uint32_t sliceSize =
		(queryComponentsGroup<CSamples1D>().size() + nThreads - 1) / nThreads;

	ComponentsGroupSlice<CSamples1D> samples =
		queryComponentsGroupSlice<CSamples1D>(Slice(sliceSize*iThread, sliceSize));


	std::random_device rd;
	std::default_random_engine gen(rd());
	std::uniform_real_distribution<float> uniform(0.0, 1.0);
	for_each([ecs, this, &gen, &uniform](HEntity hEntity,
			 CSamples1D& sample)
	{

		DVector8f iData(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f);

		assert(sample.data.size() % 8 == 0);

		float invNSamples = 1.0 / sample.data.size();
		for (uint32_t i = 0; i < sample.data.size();)
		{
			uint32_t start = i;
			uint32_t end = i + 8;
			for (; i < end; i++)
			{
				sample.data[i] = uniform(gen);
			}

			DVector8f* u = reinterpret_cast<DVector8f*>(&sample.data[start]);
			*u = minv((iData + *u)*invNSamples, DVector8f(1.0f));
			iData += 8.0f;
		}

		std::shuffle(sample.data.begin(), sample.data.end(), gen);


	}, samples);
}


void JobSamplerStratifiedGenerateSampels2D::update(WECS* ecs, uint8_t iThread)
{
	uint32_t sliceSize =
		(queryComponentsGroup<CSamples2D>().size() + nThreads - 1) / nThreads;

	ComponentsGroupSlice<CSamples2D> samples =
		queryComponentsGroupSlice<CSamples2D>(Slice(sliceSize*iThread, sliceSize));


	std::random_device rd;
	std::default_random_engine gen(rd());
	std::uniform_real_distribution<float> uniform(0.0, 1.0);

	for_each([ecs, this, &gen, &uniform](HEntity hEntity,
			 CSamples2D& samples)
	{
		DVector8f iData(0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f);

		int n = std::sqrt((float)samples.data.size());
		assert(n % 8 == 0);

		float invNSamples = 1.0 / n;


		for (uint32_t i = 0, k = 0, m = 0; i < n; i++)
		{

			for (uint32_t j = 0; j < n; j++, k++)
			{
				float invNSamples = 1.0 / samples.data.size();
				float dtX = uniform(gen);
				float dtY = uniform(gen);
				samples.data[k][0] = min((i + dtX)*invNSamples, 1.0);
				samples.data[k][1] = min((j + dtY)*invNSamples, 1.0);
			}
		}

		std::shuffle(samples.data.begin(), samples.data.end(), gen);

	}, samples);
}
WPBR_END



