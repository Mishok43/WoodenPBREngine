#pragma once
#include "pch.h"
#include "DOptions.h"

WPBR_BEGIN

class MEngine
{
public:
	MEngine();
	MEngine(const DOptions& options);

	bool init(const DOptions& options);
	void loadResources();
	void render();
private:

	bool bInit;
	std::string sceneFilename;


private:
	const std::string DEFAULT_SCENE_FILENAME = "test.scene";
};

WPBR_END