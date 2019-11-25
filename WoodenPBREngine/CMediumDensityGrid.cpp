#include "pch.h"
#include "CMediumDensityGrid.h"
#include "MEngine.h"
#include "CTextureBase.h"

WPBR_BEGIN

HEntity SMediumDensityGrid::create(const std::string& densityFile, Spectrum albedo, Spectrum scattering)
{
	MEngine& engine = MEngine::getInstance();
	HEntity hEntity = engine.createEntity();

	CTextureBinding3DR densityTexBinding;
	engine.addComponent<CTextureBinding3DR>(hEntity, densityTexBinding);
	return hEntity;
}


WPBR_END


