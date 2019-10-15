#pragma once

#include "pch.h"
#include "CBXDF.h"
#include "CMaterial.h"
#include "CSpectrum.h"
#include "CTexture.h"
#include "WoodenMathLibrarry/Utils.h"
#include "CBSDFSpecularReflectance.h"
#include "SScattering.h"
WPBR_BEGIN

struct CMaterialPerfectGlass : public CompDummy
{
	DECL_MANAGED_DENSE_COMP_DATA(CMaterialPerfectGlass, 1)
};

class SMaterialPerfectGlass
{
public:
	static HEntity create()
	{
		MEngine& engine = MEngine::getInstance();
		HEntity h = engine.createEntity();

		CTextureBindings texs;
		texs.resize(0);

		engine.addComponent<CMaterialPerfectGlass>(h);
		engine.addComponent<CTextureBindings>(h, std::move(texs));
		engine.addComponent<CBSDFRequests>(h);
		return h;
	}
};

class JobGenerateBSDFMaterialPerfectGlass : public JobParallazible
{
	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override;
};


WPBR_END


