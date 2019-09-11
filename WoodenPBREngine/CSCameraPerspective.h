#pragma once
#include "pch.h"
#include "CSCameraProjective.h"
#include "CRay.h"
#include "CRayDifferential.h"
#include "CSSampler.h"
WPBR_BEGIN

struct CCameraPerspective
{
	DVector3f dxCamera, dyCamera;
	float a;
};


class SCameraPerspective
{
	static HEntity create(CCamera camera,
						  CCameraProjective cameraProj,
						   CCameraPerspective cameraPerspective,
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
		CCameraPerspective cameraOrthographic;
		cameraOrthographic.dxCamera = rasterCamera(DVector3f(1, 0, 0)) - rasterCamera(DVector3f(0, 0, 0));
		cameraOrthographic.dyCamera = rasterCamera(DVector3f(0.0, 1.0, 0.0)) - rasterCamera(DVector3f(0, 0, 0));
		engine.addComponent<CTransformRasterCamera>(hEntity, std::move(cameraOrthographic));

		return hEntity;
	}
};

class JobCameraPerspGenerateRays : public JobParallazible
{
	static constexpr uint32_t sliceSize = 256;
	void updateNStartThreads(uint8_t nWorkThreads) override
	{
		nThreads = min(nWorkThreads, (queryComponentsGroup<CCameraSample>().size()-sliceSize+1)/sliceSize);
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		ComponentsGroup<CCameraPerspective, CCamera, CAnimatedTransform, CTransformRasterCamera> cameras =
			queryComponentsGroup<CCameraPerspective, CCamera, CAnimatedTransform, CTransformRasterCamera>();

		assert(cameras.size() == 1);

		for_each([this, ecs, iThread](HEntity,
				 const CCameraPerspective& cameraPerspe,
				 const CCamera& camera,
				 const CAnimatedTransform& world,
				 const CTransformRasterCamera& rasterCamera)
		{
			uint32_t sliceSize = (queryComponentsGroup<CCameraSamplesBatch>().size() - nThreads + 1) / nThreads;
			ComponentsGroupSlice<CCameraSamplesBatch> samples =
				queryComponentsGroupSlice<CCameraSamplesBatch>(Slice(sliceSize*iThread, sliceSize));

			for_each([&](HEntity , const CCameraSamplesBatch& samples)
			{
				for (uint32_t i = 0; i < samples.pFilm.size(); i++)
				{
					const DPoint2f& sPFilm = samples.pFilm[i];
					const float& sTime = samples.time[i];
					DPoint3f pFilm = DPoint3f(sPFilm.x(), sPFilm.y(), 0);
					DPoint3f pCamera = rasterCamera(pFilm);
					
					DRayf ray = DRayf(DPoint3f(0.0, 0.0, 0.0), normalize(DVector3f(pCamera)));
					ray.t = wml::lerp(camera.shutterOpenTime, camera.shutterCloseTime, sTime);
					ray = world(ray);

					HEntity hEntity = ecs->createEntity();
					ecs->addComponent<CRay>(hEntity, std::move(ray));
					ecs->addComponent<CSampleIndex>(hEntity, CSampleIndex{ i });
					
					//ecs->addComponent<CRay>(hEntity, std::move(ray));
				}
			}, samples);
		}, cameras);
	}
};

class JobCameraPerspGenerateRaysDifferential : public JobParallazible
{
	void updateNStartThreads(uint8_t nWorkThreads) override
	{
		nThreads = nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		ComponentsGroup<CCameraPerspective, CCamera, CAnimatedTransform, CTransformRasterCamera> cameras =
			queryComponentsGroup<CCameraPerspective, CCamera, CAnimatedTransform, CTransformRasterCamera>();

		assert(cameras.size() == 1);

		for_each([this, ecs, iThread](HEntity,
				 const CCameraPerspective& cameraPerspe,
				 const CCamera& camera,
				 const CAnimatedTransform& world,
				 const CTransformRasterCamera& rasterCamera)
		{
			uint32_t sliceSize = (queryComponentsGroup<CCameraSamplesBatch>().size() - nThreads + 1) / nThreads;
			ComponentsGroupSlice<CCameraSamplesBatch> samples =
				queryComponentsGroupSlice<CCameraSamplesBatch>(Slice(sliceSize*iThread, sliceSize));

			for_each([&](HEntity, CCameraSamplesBatch& samples)
			{
				for (uint32_t i = 0; i < samples.pFilm.size(); i++)
				{
					const DPoint2f& sPFilm = samples.pFilm[i];
					const float& sTime = samples.time[i];
					DPoint3f pFilm = DPoint3f(sPFilm.x(), sPFilm.y(), 0);
					DPoint3f pCamera = rasterCamera(pFilm);

					DRayDifferentialf ray = DRayDifferentialf(DPoint3f(0.0, 0.0, 0.0), normalize(DVector3f(pCamera));
					ray.difXRay.origin = ray.difYRay.origin = ray.origin;
					ray.difXRay.dir = normalize(DVector3f(pCamera) + cameraPerspective.dxCamera);
					ray.difYRay.dir = normalize(DVector3f(pCamera) + cameraPerspective.dyCamera);
					ray.t = wml::lerp(camera.shutterOpenTime, camera.shutterCloseTime, sample.time);
					ray = world(ray);

					HEntity hEntity = ecs->createEntity();
					ecs->addComponent<CRayDifferential>(hEntity, std::move(ray));
					samples.raysEntities[i] = hEntity;
					//ecs->addComponent<CSampleIndex>(hEntity, CSampleIndex{ i });

					//ecs->addComponent<CRay>(hEntity, std::move(ray));
				}
			}, samples);
		}, cameras);
	}
};


WPBR_END