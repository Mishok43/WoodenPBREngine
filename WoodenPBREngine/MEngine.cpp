#include "pch.h"
#include "MEngine.h"
#include "CTexture.h"
#include "CMaterial.h"
#include "CMaterialMetal.h"
#include "CMaterialDielectric.h"
#include "CMaterialPerfectGlass.h"
#include "CSSphere.h"
#include "CSFilm.h"
#include "SScattering.h"
#include "CSCameraPerspective.h"
#include "CSSamplerStratified.h"
#include "CTexture.h"
#include "SLBVH.h"
#include "CSpectrum.h"
#include "MeshTools.h"
#include "Settings.h"

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

wal::AllocatorLinear _tmpAllocator(1024*1024);

int res = RESOLUTION;


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
	HEntity hCamera = SCameraPerspective::create(std::move(camera), std::move(cameraProj), std::move(world), DTransformf::makePerspective(100.0, 1.0, 10000.0), std::move(film));
}

void MEngine::buildMaterials()
{
}

void MEngine::buildScene()
{
	buildCameraAndFilm();

	// Adding material
	HEntity hMaterialMetal = CSMaterialMetal::create("albedo.png", "roughness.png");
	HEntity hMaterialGlass = SMaterialPerfectGlass::create();
	HEntity hMaterialDielectric = CSMaterialDielectric::create();

	// Loading all textures
	JOB_RUN(JobLoadTextureRGB)

	CFilterTableGaussing filter;
	filter.size = 64;
	addComponent<CFilterTableGaussing>(createEntity(), std::move(filter));
	JOB_RUN(JobFilterTableGaussing)
		/*
		{
			CTransform p = DTransformf::makeTranslate(0.0f, 0.0f, 12.0f);
			HEntity hSphere = SSphere::create(std::move(p), CSphere(7.0));
			addComponent<CMaterialHandle>(hSphere, CMaterialHandle(hMaterialMetal));
		}*/

	{
		CTransform p = DTransformf::makeRotateX(radians(-35.0))*DTransformf::makeTranslate(0.0f, 0.0f, 40.0);
		//CTransform p = DTransformf::makeTranslate(-2.0f, -3.5f, 12.0f);
		HEntity hShape = STriangleMesh::create(p, MeshTools::createQuad(50.0f, 50.0f));
		addComponent<CMaterialHandle>(hShape, CMaterialHandle(hMaterialMetal));
	}

	{
		CTransform p =
			DTransformf::makeRotateZ(radians(180.0))*
			DTransformf::makeScale(-4.65f, 4.65f, 4.65f)*
			DTransformf::makeTranslate(-0.8f, 0.0f, 10);
		HEntity hShape = STriangleMesh::create(p, MeshTools::loadMesh("robot.obj"));
		addComponent<CMaterialHandle>(hShape, CMaterialHandle(hMaterialMetal));
	}


	//{
	//	CTransform p = DTransformf::makeTranslate(3.3f, 1.0f, 4.5f);
	//	HEntity hSphere = SSphere::create(std::move(p), CSphere(0.75));
	//	addComponent<CMaterialHandle>(hSphere, CMaterialHandle(hMaterialMetal));
	//}

	// //Adding lights
	CLight l;
	{
		CTransform p = DTransformf::makeTranslate(-0.0, -4.5f, -3.5f);
		HEntity hSphereLight = SSphere::create(std::move(p), CSphere(0.5f));


		float lambda = 600;
		float lemit;
		blackbody(&lambda, 1, 1500, &lemit);
		l.LEmit = Spectrum(lemit);
		addComponent<CLight>(hSphereLight, l);
		addComponent<CLightComputeRequests>(hSphereLight);
		addComponent<CLightSamplingRequests>(hSphereLight);
	}
	CLight l2;
	{
		CTransform p = DTransformf::makeTranslate(-0.0, -8.0f, 5.5f);
		HEntity hSphereLight = SSphere::create(std::move(p), CSphere(0.45f));


		float lambda = 600;
		float lemit;
		blackbody(&lambda, 1, 1500, &lemit);
		l2.LEmit = Spectrum(lemit*0.8);
		addComponent<CLight>(hSphereLight, l2);
		addComponent<CLightComputeRequests>(hSphereLight);
		addComponent<CLightSamplingRequests>(hSphereLight);
	}


	
	//{
	//	CTransform p = DTransformf::makeTranslate(0.0, -5.0f, -35.0f);
	//	HEntity hSphereLight = SSphere::create(std::move(p), CSphere(1.0f));


	//	float lambda = 600;
	//	float lemit;
	//	blackbody(&lambda, 1, 1500, &lemit);
	//	l.LEmit = Spectrum(lemit);
	//	addComponent<CLight>(hSphereLight, l);
	//	addComponent<CLightComputeRequests>(hSphereLight);
	//	addComponent<CLightSamplingRequests>(hSphereLight);
	//}

	/*{
		CTransform p = DTransformf::makeTranslate(-0.0, -14.0f, 30.0f);
		HEntity hSphereLight = SSphere::create(std::move(p), CSphere(1.0f));

		float lemit[nCIESamples];
		blackbody(CIE_lambda, nCIESamples, 3000, lemit);
		l.LEmit = Spectrum::fromSampled(CIE_lambda, lemit, nCIESamples);
		addComponent<CLight>(hSphereLight, l);
		addComponent<CLightComputeRequests>(hSphereLight);
		addComponent<CLightSamplingRequests>(hSphereLight);
	}*/

	//{
	//	CTransform p = DTransformf::makeTranslate(-0.0, 1.0f, -3.5f);
	//	HEntity hSphereLight = SSphere::create(std::move(p), CSphere(0.4f));

	//	float lambda = 600;
	//	float lemit;
	//	blackbody(&lambda, 1, 1500, &lemit);
	//	l.LEmit = Spectrum(lemit*1.85);
	//	addComponent<CLight>(hSphereLight, l);
	//	addComponent<CLightComputeRequests>(hSphereLight);
	//	addComponent<CLightSamplingRequests>(hSphereLight);
	//}


	/*{
		CTransform p = DTransformf::makeTranslate(-10.0, -15.0f, 10.0f);
		HEntity hSphereLight = SSphere::create(std::move(p), CSphere(1.0f));

		float lambda = 600;
		float lemit;
		blackbody(&lambda, 1, 1600, &lemit);
		l.LEmit = RGBSpectrum(DVector3f(0.0f, lemit*0.1, lemit));
		addComponent<CLight>(hSphereLight, l);
		addComponent<CLightComputeRequests>(hSphereLight);
		addComponent<CLightSamplingRequests>(hSphereLight);
	}*/

	//{
	//	CTransform p = DTransformf::makeTranslate(0.0, 3.0, 0.0);
	//	HEntity hSphereLight = SSphere::create(std::move(p), CSphere(0.25f));

	//	float lemit[nCIESamples];
	//	blackbodyNormalized(CIE_lambda, nCIESamples, 3000, lemit);
	//	l.LEmit = Spectrum::fromSampled(CIE_lambda, lemit, nCIESamples)*300000000;
	//	addComponent<CLight>(hSphereLight, l);
	//	addComponent<CLightComputeRequests>(hSphereLight);
	//	addComponent<CLightSamplingRequests>(hSphereLight);
	//}

	//{
	//	CTransform p = DTransformf::makeTranslate(-2.0, 80.0, 3.5);
	//	HEntity hSphereLight = SSphere::create(std::move(p), CSphere(1.25f));
	//
	//	float lemit[nCIESamples];
	//	blackbody(CIE_lambda, nCIESamples, 1550, lemit);
	//	l.LEmit = Spectrum::fromSampled(CIE_lambda, lemit, nCIESamples);
	//	addComponent<CLight>(hSphereLight, l);
	//	addComponent<CLightComputeRequests>(hSphereLight);
	//	addComponent<CLightSamplingRequests>(hSphereLight);
	//}

	/*{
		CTransform p = DTransformf::makeTranslate(0.0, 4.0f, 5.0f);
		HEntity hSphereLight = SSphere::create(std::move(p), CSphere(0.1f));

		CLight l;

		float lemit[nCIESamples];
		blackbody(CIE_lambda, 1, 1800, lemit);
		l.LEmit = Spectrum::fromSampled(CIE_lambda, lemit, nCIESamples);
		addComponent<CLight>(hSphereLight, l);
		addComponent<CLightComputeRequests>(hSphereLight);
		addComponent<CLightSamplingRequests>(hSphereLight);
	}*/

	//{
	//	CTransform p = DTransformf::makeTranslate(0.0, 5.0f, -150.6f);
	//	HEntity hSphereLight = SSphere::create(std::move(p), CSphere(130.0f));

	//	CLight l;

	//	float lemit[nCIESamples];
	//	blackbody(CIE_lambda, 1, 3000, lemit);
	//	l.LEmit = Spectrum::fromSampled(CIE_lambda, lemit, nCIESamples);
	//	addComponent<CLight>(hSphereLight, l);
	//	addComponent<CLightComputeRequests>(hSphereLight);
	//	addComponent<CLightSamplingRequests>(hSphereLight);
	//}


	JOB_RUN(JobTriangleMeshGenerateTriangles)
}

void MEngine::buildLBVH()
{
	JOB_RUN(JobUpdateBoundsAndCentroidSphere)
	JOB_RUN(JobUpdateBoundsAndCentroidTriangle)
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
	JOB_RUN(JobProcessTriangleInteractionRequests)
	JOB_RUN(JobProcessRayCastsResults)
	JOB_RUN(JobProcessSphereFullInteractionRequests)
	JOB_RUN(JobProcessTriangleFullInteractionRequests)
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

	for (uint32_t k = 0; k < pow(res /32, 2); k++)
	{

		JOB_RUN(JobGenerateFilmTiles)
		JOB_RUN(JobCreateCameraSamples)
		JOB_RUN(JobSamplerStratifiedGenerateSampels1D)
		JOB_RUN(JobSamplerStratifiedGenerateSampels2D)
		for (uint32_t j = 0; j < NSAMPLERS_PER_PIXEL; j++)
		{

			JOB_RUN(JobSamplerUpdateCameraSamples)
			JOB_RUN(JobCameraPerspGenerateRaysDifferential)
			runCollisionSystem();
			JOB_RUN(JobScatteringAccumulateEmittedLight)
			JOB_RUN(JobComputeDifferentialsForSurfInter)
			JOB_RUN(JobMapUVRequestsGenerate)
			JOB_RUN(JobSphereProcessMapUVRequests)
			JOB_RUN(JobTriangleProcessMapUVRequests)

			JOB_RUN(JobGenerateBSDFRequests)
			JOB_RUN(JobGenerateBSDFMaterialMetal)
			JOB_RUN(JobGenerateBSDFMaterialDielectric)
			JOB_RUN(JobGenerateBSDFMaterialPerfectGlass)

			JOB_RUN(JobBSDFComputeTransform)
			JOB_RUN(JobScatteringSampleLight)
			for (uint32_t i = 0; i < 1; i++)
			{
				JOB_RUN(JobScatteringSampleLightLI)
					JOB_RUN(JobSphereLightProcessSamplingRequests)

					JOB_RUN(JobScatteringCastShadowRays)
					runCollisionSystem();
				JOB_RUN(JobScatteringProcessShadowRay)
					JOB_RUN(JobBSDFConductorMicrofaceCompute)
					JOB_RUN(JobBSDFSpecularReflectionCompute)
					JOB_RUN(JobBSDFDielectricMicrofaceCompute)

					JOB_RUN(JobScatteringIntegrateImportanceLight)

					JOB_RUN(JobScatteringSampleBSDF)
					JOB_RUN(JobBSDFConductorMicrofaceSample)
					JOB_RUN(JobBSDFSpecularReflectionSample)
					JOB_RUN(JobBSDFDielectricMicrofaceSample)

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
