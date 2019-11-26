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

	HEntity getEnvLight() const;
private:

	bool bInit;
	std::string sceneFilename;

private:
	const std::string DEFAULT_SCENE_FILENAME = "test.scene";

	void buildCameraAndFilm();
	void buildLBVH();
	void buildScene();
	void preprocess();
	void buildMaterials();
	void runCollisionSystem();

protected:
	HEntity hEnvLight;
};

WPBR_END