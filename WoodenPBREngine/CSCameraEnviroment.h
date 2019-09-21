#pragma once
#include "pch.h"
#include "CSCamera.h"

WPBR_BEGIN

struct CCameraEnviroment
{
	DVector3f dxCamera, dyCamera;
};

class SCameraEnviroment
{
	static HEntity create(CCamera camera,
						  DAnimatedTransformf world,
						  CFilm film,
						  CMedium medium)
	{
		HEntity hEntity = SCamera::create(std::move(camera), std::move(world), std::move(film), std::move(medium));

		MEngine& engine = MEngine::getInstance();
		CCameraEnviroment cameraEnviroment;
		engine.addComponent<CCameraEnviroment>(hEntity, std::move(cameraEnviroment));


		return hEntity;
	}
};

class JobCameraEnvirGenerateRays : public JobParallazible
{
	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		ComponentsGroup<CCameraEnviroment, CCamera, CAnimatedTransform, CCameraSamples, CFilm> samples =
			queryComponentsGroup<CCameraEnviroment, CCamera, CAnimatedTransform, CCameraSamples, CFilm>();
		for_each([ecs, iThread](HEntity hEntity,
				 const CCameraEnviroment& cameraEnviroment,
				 const CCamera& camera,
				 const CAnimatedTransform& world,
				 const CCameraSamples& samples,
				 const CFilm& film)
		{
			uint16_t sliceSize = (samples.size() + iThread - 1) / iThread;
			uint16_t iStart = sliceSize * iThread;
			for (uint16_t i = iStart; i < iStart + sliceSize; i++)
			{
				const CCameraSample& sample = samples[i];

				float theta = PI * sample.pFilm.y() / film.fullResolution.y();
				float phi = 2 * PI * sample.pFilm.x() / film.fullResolution.x();
				DVector3f dir(std::sin(theta) * std::cos(phi), std::cos(theta),
							 std::sin(theta) * std::sin(phi));
			

				DRayf ray = DRayf(DPoint3f(0.0, 0.0, 0.0), dir, INFINITY);
				ray.t = wml::lerp(camera.shutterOpenTime, camera.shutterCloseTime, sample.time);
				ray = world(ray);
			}
		}, samples);
	}
};

class JobCameraEnvirGenerateRaysDifferential : public JobParallazible
{
	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		ComponentsGroup<CCamera, CTransform, CTransformRasterCamera, CCameraEnviroment, CCameraSamples> samples =
			queryComponentsGroup<CCamera, CTransform, CTransformRasterCamera, CCameraEnviroment, CCameraSamples>();
		for_each([ecs, iThread](HEntity hEntity,
				 const CCamera& camera,
				 const CTransform& world,
				 const CTransformRasterCamera& rasterCamera,
				 const CCameraEnviroment& cameraEnviroment, const CCameraSamples& samples)
		{
			uint16_t sliceSize = (samples.size() + iThread - 1) / iThread;
			uint16_t iStart = sliceSize * iThread;
			for (uint16_t i = iStart; i < iStart + sliceSize; i++)
			{
				const CCameraSample& sample = samples[i];

				DPoint3f pFilm = DPoint3f(sample.pFilm.x(), sample.pFilm.y(), 0);
				DPoint3f pCamera = rasterCamera(pFilm);

				DRayDifferentialf ray = DRayDifferentialf(std::move(pCamera), DVector3f(0.0, 0.0, 1.0f));
				ray.difXRay.origin = ray.origin + cameraEnviroment.dxCamera;
				ray.difYRay.origin = ray.origin + cameraEnviroment.dyCamera;
				ray.difXRay.dir = ray.difYRay.dir = ray.dir;


				ray.t = wml::lerp(camera.shutterOpenTime, camera.shutterCloseTime, sample.time);
				ray = world(ray);
			}
		}, samples);
	}
};


WPBR_END



