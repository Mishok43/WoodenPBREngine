#pragma once
#include "pch.h"
#include "MEngine.h"
#include "CAnimatedTransform.h"
#include "CSFilm.h"
#include "CMedium.h"
#include "CRayDifferential.h"
#include "WoodenECS/Job.h"

WPBR_BEGIN
struct CCamera
{
	float shutterOpenTime, shutterCloseTime;

	DECL_MANAGED_DENSE_COMP_DATA(CCamera, 1)
}; DECL_OUT_COMP_DATA(CCamera)




class SCamera
{
public:
	static HEntity create(CCamera camera,
						   DAnimatedTransformf animatedTransform,
						   CFilm film,
						   CMedium medium)
	{
		MEngine& ecs = MEngine::getInstance();
		HEntity hEntity = ecs.createEntity();
		ecs.addComponent<CCamera>(hEntity, std::move(camera));
		ecs.addComponent<CAnimatedTransform>(hEntity, std::move(animatedTransform));
		ecs.addComponent<CFilm>(hEntity, std::move(film));
		ecs.addComponent<CMedium>(hEntity, std::move(medium));
		return hEntity;
	}
};


WPBR_END


//class JobGenerateRaysDifferential : public JobParallazible
//{
//	void updateNStartThreads(uint8_t nWorkThreads) override
//	{
//		nThreads = nWorkThreads;
//	}
//
//	void update(WECS* ecs, uint8_t iThread) override
//	{
//		ComponentsGroup<CHCameraOrthographic, CCameraSamples> samples =
//			queryComponentsGroup<CHCameraOrthographic, CCameraSamples>();
//		for_each([ecs, iThread](HEntity hEntity,
//				 const CHCameraOrthographic& cameraOrthographic, const CCameraSamples& samples)
//		{
//			uint16_t sliceSize = (samples.size() + iThread - 1) / iThread;
//			uint16_t iStart = sliceSize * iThread;
//			for (uint16_t i = iStart; i < iStart + sliceSize; i++)
//			{
//				const CCameraSample& sample = samples[i];
//
//				float contributionFactor;
//				CRayDifferential rayDifferential = generateRay(sample, contributionFactor);
//
//				CCameraSample sampleDX = sample;
//				sampleDX.pFilm.x()++;
//				float contributionFactorDX;
//				rayDifferential.difXRay = generateRay(sampleDX, contributionFactorDX);
//
//				CCameraSample sampleDY = sample;
//				sampleDY.pFilm.y()++;
//				float contributionFactorDY;
//				rayDifferential.difYRay = generateRay(sampleDY, contributionFactorDY);
//			}
//
//		}, samples);
//	}
//};
