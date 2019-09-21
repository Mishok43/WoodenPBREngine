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
}; DECL_OUT_COMP_DATA(CCameraProjective)


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
		ecs.addComponent<CTransformCameraScreen>(hEntity, std::move(cameraScreen));

		DTransformf screenRaster = 
			DTransformf.makeScale(film.fullResolution.x(),
								  film.fullResolution.y(), 1)*
			DTransformf.makeScale(1.0 / (cameraProj.screenWindow.pMax.x() - cameraProj.screenWindow.pMin.x()),
								  1.0 / (cameraProj.screenWindow.pMax.y() - cameraProj.screenWindow.pMin.y()),
								  1.0) *
			DTransformf.makeTranslate(-cameraProj.screenWindow.pMin.x(),
									  -cameraProj.screenWindow.pMin.y(),
									  0);
		ecs.addComponent<CTransformScreenRaster>(hEntity, std::move(screenRaster));
		DTransformf rasterCamera = inverse(screenRaster) * inverse(cameraScreen);
		ecs.addComponent<CTransformRasterCamera>(hEntity, std::move(rasterCamera));
		return hEntity;
	}
};

WPBR_END