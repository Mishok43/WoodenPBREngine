#pragma once
#include "pch.h"
#include "MEngine.h"
#include "CCamera.h"
#include "CTransform.h"
#include "CFilm.h"

WPBR_BEGIN



class SCamera
{
public:
	static HEntity create(CCamera camera,
						   CTransform world,
						   CFilm film)
	{
		MEngine& ecs = MEngine::getInstance();
		HEntity hEntity = ecs.createEntity();
		ecs.addComponent<CCamera>(hEntity, std::move(camera));
		ecs.addComponent<CTransform>(hEntity, std::move(world));
		ecs.addComponent<CFilm>(hEntity, std::move(film));
		return hEntity;
	}
};


WPBR_END


//class JobGenerateRaysDifferential : public JobParallazible
//{
//	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
//	{
//		return nWorkThreads;
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
