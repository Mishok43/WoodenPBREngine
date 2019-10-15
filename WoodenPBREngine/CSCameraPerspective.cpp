#include "pch.h"
#include "CSCameraPerspective.h"
#include <iostream>

WPBR_BEGIN
DECL_OUT_COMP_DATA(CCameraPerspective)


void JobCameraPerspGenerateRaysDifferential::update(WECS* ecs, uint8_t iThread) 
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
		uint32_t sliceSize = (queryComponentsGroup<CCameraSample>().size() - nThreads + 1) / getNumThreads();
		ComponentsGroupSlice<CCameraSample> samples =
			queryComponentsGroupSlice<CCameraSample>(Slice(sliceSize*iThread, sliceSize));

		for_each([&](HEntity hSample, CCameraSample& sample)
		{

			const DPoint2f& sPFilm = sample.pFilm;
			DPoint3f pFilm = DPoint3f(sPFilm.x(), sPFilm.y(), 0);
			DPoint3f pCamera = rasterCamera(pFilm);

			//std::cout << pCamera.x() << " " << pCamera.y() << std::endl;


			DRayDifferentialf ray = DRayDifferentialf(DPoint3f(0.0, 0.0, 0.0), normalize(DVector3f(pCamera)));
			ray.difXRay.origin = ray.difYRay.origin = ray.origin;
			ray.difXRay.dir = normalize(DVector3f(pCamera) + cameraPerspe.dxCamera);
			ray.difYRay.dir = normalize(DVector3f(pCamera) + cameraPerspe.dyCamera);
			//ray.t = wml::lerp(camera.shutterOpenTime, camera.shutterCloseTime, sample.time);
			ray = world(ray);

			ecs->addComponent<CRayDifferential>(hSample, ray);

			CRayCast rayCast;
			rayCast.ray = ray;
			rayCast.bSurfInteraction = true;
			ecs->addComponent<CRayCast>(hSample, std::move(rayCast));
			//ecs->addComponent<CSampleIndex>(hEntity, CSampleIndex{ i });
			//ecs->addComponent<CRay>(hEntity, std::move(ray));

		}, samples);
	}, cameras);
}


WPBR_END

