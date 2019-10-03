#pragma once
#include "pch.h"
#include "DOptions.h"
#include "WoodenECS/WECS.h"

WPBR_BEGIN

class MEngine: public wecs::WECS
{
public:
	static MEngine& getInstance()
	{
		static MEngine engine;
		return engine;
	}

	void loadResources();
	void render();
private:

	bool bInit;
	std::string sceneFilename;

private:
	const std::string DEFAULT_SCENE_FILENAME = "test.scene";

	void buildCameraAndFilm();
	void buildLBVH();
	void buildScene();
	void buildMaterials();
	void runCollisionSystem();
};

WPBR_END