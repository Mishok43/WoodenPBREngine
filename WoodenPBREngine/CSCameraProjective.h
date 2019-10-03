#pragma once
#include "pch.h"
#include "CSCamera.h"
#include "CTransform.h"
#include "WoodenMathLibrarry/DBounds.h"
#include "WoodenMathLibrarry/DTransform.h"

WPBR_BEGIN

struct CCameraProjective
{
	DBounds2f screenWindow;
	float lensr, focald;

	DECL_MANAGED_DENSE_COMP_DATA(CCameraProjective, 1)
}; 


class SCameraProjective
{
public:
	static HEntity create(CCamera camera,
						   CCameraProjective cameraProj,
						   CTransform world,
						   CTransform cameraScreen,
						   CFilm film)
	{
		MEngine& ecs = MEngine::getInstance();
		HEntity hEntity = SCamera::create(std::move(camera),
										  std::move(world),
										  std::move(film));
		ecs.addComponent<CCameraProjective>(hEntity, std::move(cameraProj));		
		ecs.addComponent<CTransformCameraScreen>(hEntity, CTransform(cameraScreen));

		DTransformf screenRaster = 
			DTransformf::makeTranslate(-cameraProj.screenWindow.pMin.x(),
									  -cameraProj.screenWindow.pMin.y(),
									  0)*
			DTransformf::makeScale(1.0 / (cameraProj.screenWindow.pMax.x() - cameraProj.screenWindow.pMin.x()),
								   1.0 / (cameraProj.screenWindow.pMax.y() - cameraProj.screenWindow.pMin.y()),
								   1.0) *
			DTransformf::makeScale(film.resolution.x(),
								   film.resolution.y(), 1);
		ecs.addComponent<CTransformScreenRaster>(hEntity, CTransform(screenRaster));
		DTransformf rasterScreen = inverse(screenRaster);
		DTransformf screenCamera = inverse(cameraScreen);
		DTransformf rasterCamera = rasterScreen  * screenCamera;

		auto m = inverse(rasterCamera.m());
		ecs.addComponent<CTransformRasterCamera>(hEntity, std::move(rasterCamera));
		return hEntity;
	}
};

WPBR_END