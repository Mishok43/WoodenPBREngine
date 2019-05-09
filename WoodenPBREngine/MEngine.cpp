#include "MEngine.h"


WPBR_BEGIN

MEngine::MEngine(const DOptions& options)
{
	assert(init(options), "PBR Engine wasn't initialized successfully");
}


bool MEngine::init(const DOptions& options)
{
	if (bInit)
	{
		return false;
	}

	sceneFilename = options.inSceneFileName;

	bInit = true;
	return false;
}

void MEngine::loadResources()
{
	if (sceneFilename.size() > 0)
	{
		sceneFilename = DEFAULT_SCENE_FILENAME;
	}




}

void MEngine::render()
{

}

WPBR_END
