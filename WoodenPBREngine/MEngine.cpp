#include "pch.h"
#include "MEngine.h"
#include "CTexture.h"
#include "CMaterial.h"
#include "CMaterialMatte.h"
#include "CSSphere.h"
#include "CSCameraProjective.h"
#include "CSSamplerStratified.h"
#include "CTexture.h"
#include "SLBVH.h"


WPBR_BEGIN

bool MEngine::init(const DOptions& options)
{
	if (bInit)
	{
		return false;
	}

	if (options.inSceneFileName.size() > 0)
	{
		sceneFilename = options.inSceneFileName;
	}
/*
	registerComponent<class CLight>();
	registerComponent<class CPrimitive>();
	registerComponent<class CRay>();
	registerSystem<class SLighting>();
*/
	bInit = true;
	return false;
}

#define JOB_RUN(JobT) JobT().run(this);


void MEngine::buildCameraAndFilm()
{
	CFilm film;
	film.resolution = DBounds2i(DPoint2i(0, 0), DPoint2i(256, 256));
	film.rgbOutput.resize(film.resolution.area() * 3);
	film.outFile = "test.png";
	film.nProcessedTiles = 0;

	CCamera camera; camera.shutterCloseTime = 1.0f; camera.shutterOpenTime = 0.0f;
	CCameraProjective cameraProj; cameraProj.screenWindow = DBounds2f(DPoint2f(0.0f, 0.0f), DPoint2f(1.0, 1.0));
	CTransform world;
	CTransform perp = DTransform::makePerspective(45.0, 1.0, 1000.0);

	HEntity hCamera = SCameraPerspective::create(std::move(camera), std::move(cameraProj), std::move(world), std::move(perp), std::move(film));
}

void MEngine::buildMaterials()
{
	
}

void MEngine::buildScene()
{
	buildCameraAndFilm();

	// Adding material
	HEntity hMaterial = CSMaterialMatte::create("diff.png", "roughness.png");

	// Loading all textures
	JOB_RUN(JobLoadTextureRGB)
	// Adding primitives
	{
		CTransform p = DTransformf::makeTranslate(0.0f, 0.0f, 20.0f);
		HEntity hSphere = SSphere::create(std::move(p), CSphere(5.0));
		addComponent<CMaterialHandle>(hSphere, hMaterial);
	}

	// Adding lights
	{
		CTransform p = DTransformf::makeTranslate(0.0f, 15.0f, 25.0f);
		HEntity hSphereLight = SSphere::create(std::move(p), CSphere(2.0));

		CLight l;
		addComponent<CLight>(hSphereLight, l);
		addComponent<CLightComputeRequests>(hSphereLight);
		addComponent<CLightSamplingRequests>(hSphereLight);
	}
}

void MEngine::buildLBVH()
{
	JOB_RUN(JobUpdateBoundsAndCentroidSphere)
	JOB_RUN(JobGenerateShapesCentroidBound)
	JOB_RUN(JobGenerateShapesMortonCode)
	JOB_RUN(JobSortsShapesByMortonCode)
	JOB_RUN(JobEmitLBVH)
	JOB_RUN(JobBuildUpperSAH)
}

void MEngine::runCollisionSystem()
{
	JOB_RUN(JobProcessRayCasts)
	JOB_RUN(JobProcessRayCastsResults)
}

void MEngine::loadResources()
{

	buildScene();
	buildLBVH();

	JOB_RUN(JobGenerateFilmTiles)
	JOB_RUN(JobCreateCameraSamples)
	JOB_RUN(JobSamplerStratifiedGenerateSampels1D)
	JOB_RUN(JobSamplerStratifiedGenerateSampels2D)
	JOB_RUN(JobSamplerUpdateCameraSamples)
	JOB_RUN(JobCameraPerspGenerateRaysDifferential) 
	runCollisionSystem();
	JOB_RUN(JobScatteringAccumulateEmittedLight)
	JOB_RUN(JobComputeDifferentialsForSurfInter)
	JOB_RUN(JobSphereProcessMapUVRequests)
	JOB_RUN(JobGenerateBSDFRequests)
	JOB_RUN(JobGenerateBSDFMaterialMatte)
	for (uint32_t i = 0; i < 32; i++)
	{
		JOB_RUN(JobScatteringSampleLight)
		JOB_RUN(JobScatteringSampleLightLI)
		JOB_RUN(JobSphereLightProcessSamplingRequests)

		JOB_RUN(JobScatteringCastShadowRays)
		runCollisionSystem();
		JOB_RUN(JobScatteringProcessShadowRay)
		JOB_RUN(JobBSDFConductorMicrofaceCompute)

		JOB_RUN(JobScatteringIntegrateImportanceLight)

		JOB_RUN(JobScatteringSampleBSDF)
		JOB_RUN(JobBSDFConductorMicrofaceSample)

		JOB_RUN(JobScatteringCastShadowRaysWithInteraction)
		runCollisionSystem();
		JOB_RUN(JobScatteringProcessShadowRayWithInteraction)

		JOB_RUN(JobSphereLightProcessComputeRequests)
		JOB_RUN(JobScatteringIntegrateImportanceBSDF)
	}
	JOB_RUN(JobAccumalateLIFromSamples)
	JOB_RUN(JobOutputFilmTitles)

	JOB_RUN(JobOutputFilm)


}

void MEngine::render()
{

}

WPBR_END
