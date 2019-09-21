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
};


class SCameraPerspective
{
public:
	static HEntity create(CCamera camera,
						  CCameraProjective cameraProj,
						   CTransform world,
						   CTransform cameraScreen,
						   CFilm film)
	{
		HEntity hEntity = SCameraProjective::create(std::move(camera), std::move(cameraProj),
													std::move(world), std::move(cameraScreen), std::move(film));

		MEngine& engine = MEngine::getInstance();

		const CTransformRasterCamera& rasterCamera = engine.getComponent<CTransformRasterCamera>(hEntity);
		CCameraPerspective cameraPersp;
		cameraPersp.dxCamera = rasterCamera(DVector3f(1, 0, 0)) - rasterCamera(DVector3f(0, 0, 0));
		cameraPersp.dyCamera = rasterCamera(DVector3f(0.0, 1.0, 0.0)) - rasterCamera(DVector3f(0, 0, 0));
		engine.addComponent<CCameraPerspective>(hEntity, std::move(cameraPersp));

		return hEntity;
	}
};

class JobCameraPerspGenerateRays : public JobParallazible
{
	static constexpr uint32_t sliceSize = 256;
	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return min(nWorkThreads, (queryComponentsGroup<CCameraSample>().size()-sliceSize+1)/sliceSize);
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		ComponentsGroup<CCameraPerspective, CCamera, CTransform, CTransformRasterCamera> cameras =
			queryComponentsGroup<CCameraPerspective, CCamera, CTransform, CTransformRasterCamera>();

		assert(cameras.size() == 1);

		for_each([this, ecs, iThread](HEntity,
				 const CCameraPerspective& cameraPerspe,
				 const CCamera& camera,
				 const CTransform& world,
				 const CTransformRasterCamera& rasterCamera)
		{
			uint32_t sliceSize = (queryComponentsGroup<CCameraSample>().size() - nThreads + 1) /getNumThreads();
			ComponentsGroupSlice<CCameraSample> samples =
				queryComponentsGroupSlice<CCameraSample>(Slice(sliceSize*iThread, sliceSize));

			for_each([&](HEntity hSample, const CCameraSample& sample)
			{
				
				const DPoint2f& sPFilm = sample.pFilm;
					//const float& sTime = samples.time[i];
				DPoint3f pFilm = DPoint3f(sPFilm.x(), sPFilm.y(), 0);
				DPoint3f pCamera = rasterCamera(pFilm);
					
				DRayf ray = DRayf(DPoint3f(0.0, 0.0, 0.0), normalize(DVector3f(pCamera)));
					//ray.t = wml::lerp(camera.shutterOpenTime, camera.shutterCloseTime, sTime);
				ray = world(ray);
				ecs->addComponent<CRay>(hSample, std::move(ray));

				CRayCast rayCast;
				rayCast.ray = ray;
				ecs->addComponent<CRayCast>(hSample, std::move(rayCast));
			}, samples);
		}, cameras);
	}
};

class JobCameraPerspGenerateRaysDifferential : public JobParallazible
{
	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		ComponentsGroup<CCameraPerspective, CCamera, CTransform, CTransformRasterCamera> cameras =
			queryComponentsGroup<CCameraPerspective, CCamera, CTransform, CTransformRasterCamera>();

		assert(cameras.size() == 1);

		for_each([this, ecs, iThread](HEntity,
				 const CCameraPerspective& cameraPerspe,
				 const CCamera& camera,
				 const CTransform& world,
				 const CTransformRasterCamera& rasterCamera)
		{
			uint32_t sliceSize = (queryComponentsGroup<CCameraSample>().size() - nThreads + 1) /getNumThreads();
			ComponentsGroupSlice<CCameraSample> samples =
				queryComponentsGroupSlice<CCameraSample>(Slice(sliceSize*iThread, sliceSize));

			for_each([&](HEntity hSample, CCameraSample& sample)
			{
				
					const DPoint2f& sPFilm = sample.pFilm;
					DPoint3f pFilm = DPoint3f(sPFilm.x(), sPFilm.y(), 0);
					DPoint3f pCamera = rasterCamera(pFilm);

					DRayDifferentialf ray = DRayDifferentialf(DPoint3f(0.0, 0.0, 0.0), normalize(DVector3f(pCamera)));
					ray.difXRay.origin = ray.difYRay.origin = ray.origin;
					ray.difXRay.dir = normalize(DVector3f(pCamera) + cameraPerspe.dxCamera);
					ray.difYRay.dir = normalize(DVector3f(pCamera) + cameraPerspe.dyCamera);
					//ray.t = wml::lerp(camera.shutterOpenTime, camera.shutterCloseTime, sample.time);
					ray = world(ray);

					ecs->addComponent<CRayDifferential>(hSample, std::move(ray));

					CRayCast rayCast;
					rayCast.ray = ray;
					ecs->addComponent<CRayCast>(hSample, std::move(rayCast));
					//ecs->addComponent<CSampleIndex>(hEntity, CSampleIndex{ i });
					//ecs->addComponent<CRay>(hEntity, std::move(ray));
				
			}, samples);
		}, cameras);
	}
};


WPBR_END