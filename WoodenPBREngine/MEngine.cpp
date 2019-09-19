#include "pch.h"
#include "MEngine.h"

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


void buildScene()
{
	JOB_RUN(JobLoadTextureRGB)
}

#define JOB_RUN(JobT) JobT().run(this);
void buildLBVH()
{
	JOB_RUN(JobUpdateBoundsAndCentroidSphere)
	JOB_RUN(JobGenerateShapesCentroidBound)
	JOB_RUN(JobGenerateShapesMortonCode)
	JOB_RUN(JobSortsShapesByMortonCode)
	JOB_RUN(JobEmitLBVH)
	JOB_RUN(JobBuildUpperSAH)
}

void runCollisionSystem()
{
	JOB_RUN(JobProcessRayCasts)
	JOB_RUN(JobProcessRayCastsResults)
}

void MEngine::loadResources()
{

	buildScene();
	buildLBVH();

	JOB_RUN(JobGenerateFilmTiles())
	JOB_RUN(JobCreateCameraSamples())
	JOB_RUN(JobSamplerStratifiedGenerateSampels1D)
	JOB_RUN(JobSamplerStratifiedGenerateSampels2D)
	JOB_RUN(JobSamplerUpdateCameraSamples)
	JOB_RUN(JobCameraPerspGenerateRaysDifferential) 
	runCollisionSystem();
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
