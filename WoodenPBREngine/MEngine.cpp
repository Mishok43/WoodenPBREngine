#include "pch.h"
#include "MEngine.h"
#include "CTexture.h"
#include "CMaterial.h"
#include "CMaterialMatte.h"
#include "CSSphere.h"
#include "CSFilm.h"
#include "SScattering.h"
#include "CSCameraPerspective.h"
#include "CSSamplerStratified.h"
#include "CTexture.h"
#include "SLBVH.h"
#include "CSpectrum.h"


WPBR_BEGIN

//bool MEngine::init(const DOptions& options)
//{
//	if (bInit)
//	{
//		return false;
//	}
//
//	if (options.inSceneFileName.size() > 0)
//	{
//		sceneFilename = options.inSceneFileName;
//	}
///*
//	registerComponent<class CLight>();
//	registerComponent<class CPrimitive>();
//	registerComponent<class CRay>();
//	registerSystem<class SLighting>();
//*/
//	bInit = true;
//	return false;
//}

#define JOB_RUN(JobT)\
{\
 auto j = JobT();\
 j.setAllocatorTemp(&_tmpAllocator);\
 j.run(this);\
 _tmpAllocator.reset();\
}

wal::AllocatorLinear _tmpAllocator(64*1024);

int res = 512;


void MEngine::buildCameraAndFilm()
{
	CFilm film;
	film.resolution = DPoint2i(res, res);
	film.rgbOutput.resize(film.resolution.x()*film.resolution.y()* 3);
	film.outFile = "test.ppm";
	film.nProcessedTiles = 0;

	CCamera camera; camera.shutterCloseTime = 1.0f; camera.shutterOpenTime = 0.0f;
	CCameraProjective cameraProj; cameraProj.screenWindow = DBounds2f(DPoint2f(-1.0f, -1.0f), DPoint2f(1.0, 1.0));
	CTransform world;
	HEntity hCamera = SCameraPerspective::create(std::move(camera), std::move(cameraProj), std::move(world), DTransformf::makePerspective(120.0, 1.0, 10000.0), std::move(film));
}

void MEngine::buildMaterials()
{
	
}

void MEngine::buildScene()
{
	buildCameraAndFilm();

	// Adding material
	HEntity hMaterial = CSMaterialMatte::create("albedo.png", "roughness.png");

	// Loading all textures
	JOB_RUN(JobLoadTextureRGB)

	CFilterTableGaussing filter;
	filter.size = 48;
	addComponent<CFilterTableGaussing>(createEntity(), std::move(filter));
	JOB_RUN(JobFilterTableGaussing)
	// Adding primitives
	{
		CTransform p = DTransformf::makeTranslate(2.0f, 0.0f, 12.0f);
		HEntity hSphere = SSphere::create(std::move(p), CSphere(8.0));
		addComponent<CMaterialHandle>(hSphere, CMaterialHandle(hMaterial));
	}

	// Adding lights
	{
		CTransform p = DTransformf::makeTranslate(-1.0, 2.0f, 3.0f);
		HEntity hSphereLight = SSphere::create(std::move(p), CSphere(0.15f));

		CLight l;

		float lambdas = 600.0;
		float lemit;
		blackbody(&lambdas, 1, 1500, &lemit);
		l.LEmit = Spectrum(lemit);
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
	JOB_RUN(JobProcessSphereSurfInteractionRequests)
	JOB_RUN(JobProcessRayCastsResults)
	JOB_RUN(JobProcessSphereFullInteractionRequests)
	JOB_RUN(JobCollisionFinish)
}

void MEngine::loadResources()
{
	_tmpAllocator.init();

	SampledSpectrum::init();
	buildScene();
	buildLBVH();


}

void MEngine::render()
{

	for (uint32_t k = 0; k < pow(res /16, 2); k++)
	{

		JOB_RUN(JobGenerateFilmTiles)
			JOB_RUN(JobCreateCameraSamples)
			JOB_RUN(JobSamplerStratifiedGenerateSampels1D)
			JOB_RUN(JobSamplerStratifiedGenerateSampels2D)
			for (uint32_t j = 0; j < 16; j++)
			{
				JOB_RUN(JobSamplerUpdateCameraSamples)
					JOB_RUN(JobCameraPerspGenerateRaysDifferential)
					runCollisionSystem();
				JOB_RUN(JobScatteringAccumulateEmittedLight)
					JOB_RUN(JobComputeDifferentialsForSurfInter)
					JOB_RUN(JobMapUVRequestsGenerate)
					JOB_RUN(JobSphereProcessMapUVRequests)
					JOB_RUN(JobGenerateBSDFRequests)
					JOB_RUN(JobGenerateBSDFMaterialMatte)
					JOB_RUN(JobScatteringSampleLight)
					for (uint32_t i = 0; i < 32; i++)
					{
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
				JOB_RUN(JobScatteringFinish)
				JOB_RUN(JobAccumalateLIFromSamples)
			}

		JOB_RUN(JobOutputFilmTitles)
	}
		JOB_RUN(JobOutputFilm)
}

WPBR_END
