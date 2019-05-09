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

	registerComponent<class CLight>();
	registerComponent<class CPrimitive>();
	registerComponent<class CRay>();
	registerSystem<class SLighting>();

	bInit = true;
	return false;
}

void MEngine::loadResources()
{
	static_assert(false);
	DScene scene;// TODO: PARSE SCENE FILE!
}

void MEngine::render()
{

}

WPBR_END
