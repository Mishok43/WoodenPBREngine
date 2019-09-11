#pragma once
#include "pch.h"
#include "CSCameraProjective.h"

WPBR_BEGIN

struct CCameraOrthographic
{
	DVector3f dxCamera, dyCamera;
	float lensRadius;
};

class SCameraOrthographic
{
	static HEntity create(CCamera camera,
						   CCameraProjective cameraProj,
						   DAnimatedTransformf world,
						   DTransformf cameraScreen,
						   CFilm film,
						   CMedium medium)
	{
		HEntity hEntity = SCameraProjective::create(std::move(camera), std::move(cameraProj),
								  std::move(world), std::move(cameraScreen), std::move(film),
								  std::move(medium));

		MEngine& engine = MEngine::getInstance();

		const CTransformRasterCamera& rasterCamera = engine.getComponent<CTransformRasterCamera>(hEntity);
		CCameraOrthographic cameraOrthographic;
		cameraOrthographic.dxCamera = rasterCamera(DVector3f(1, 0, 0));
		cameraOrthographic.dyCamera = rasterCamera(DVector3f(0.0, 1.0, 0.0));
		engine.addComponent<CCameraOrthographic>(hEntity, std::move(cameraOrthographic));


		return hEntity;
	}
};

class JobCameraOrthoGenerateRays : public JobParallazible
{
	void updateNStartThreads(uint8_t nWorkThreads) override
	{
		nThreads = nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		ComponentsGroup<CCameraOrthographic, CCamera, CAnimatedTransform, CTransformRasterCamera, CCameraSamples> samples =
			queryComponentsGroup<CCameraOrthographic, CCamera, CAnimatedTransform, CTransformRasterCamera, CCameraSamples>();
		for_each([ecs, iThread](HEntity hEntity,
				 const CCameraOrthographic& cameraOrthographic,
				 const CCamera& camera,
				 const CAnimatedTransform& world,
				 const CTransformRasterCamera& rasterCamera,
				 const CCameraSamples& samples)
		{
			uint16_t sliceSize = (samples.size() + iThread - 1) / iThread;
			uint16_t iStart = sliceSize * iThread;
			for (uint16_t i = iStart; i < iStart + sliceSize; i++)
			{
				const CCameraSample& sample = samples[i];

				DPoint3f pFilm = DPoint3f(sample.pFilm.x(), sample.pFilm.y(), 0);
				DPoint3f pCamera = rasterCamera(pFilm);

				DRayf ray = DRayf(std::move(pCamera), DVector3f(0.0, 0.0, 1.0f));
				ray.t = wml::lerp(camera.shutterOpenTime, camera.shutterCloseTime, sample.time);
				ray = world(ray);
			}
		}, samples);
	}
};

class JobCameraOrthoGenerateRaysDifferential : public JobParallazible
{
	void updateNStartThreads(uint8_t nWorkThreads) override
	{
		nThreads = nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		ComponentsGroup<CCameraOrthographic, CCamera, CAnimatedTransform, CTransformRasterCamera, CCameraSamples> samples =
			queryComponentsGroup<CCameraOrthographic, CCamera, CAnimatedTransform, CTransformRasterCamera, CCameraSamples>();
		for_each([ecs, iThread](HEntity hEntity,
				 const CCameraOrthographic& cameraOrthographic,
				 const CCamera& camera,
				 const CAnimatedTransform& world,
				 const CTransformRasterCamera& rasterCamera,
				 const CCameraSamples& samples)
		{
			uint16_t sliceSize = (samples.size() + iThread - 1) / iThread;
			uint16_t iStart = sliceSize * iThread;
			for (uint16_t i = iStart; i < iStart + sliceSize; i++)
			{
				const CCameraSample& sample = samples[i];

				DPoint3f pFilm = DPoint3f(sample.pFilm.x(), sample.pFilm.y(), 0);
				DPoint3f pCamera = rasterCamera(pFilm);

				DRayDifferentialf ray = DRayDifferentialf(std::move(pCamera), DVector3f(0.0, 0.0, 1.0f));
				ray.difXRay.origin = ray.origin + cameraOrthographic.dxCamera;
				ray.difYRay.origin = ray.origin + cameraOrthographic.dyCamera;
				ray.difXRay.dir = ray.difYRay.dir = ray.dir;


				ray.t = wml::lerp(camera.shutterOpenTime, camera.shutterCloseTime, sample.time);
				ray = world(ray);
			}
		}, samples);
	}
};


WPBR_END