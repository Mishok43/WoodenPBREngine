#pragma once

#include "pch.h"
#include "CBXDF.h"
#include "CMaterial.h"
#include "CSpectrum.h"
#include "CTexture.h"
#include "WoodenMathLibrarry/Utils.h"
#include "CBSDFDielectricMicroface.h"
#include "SScattering.h"
WPBR_BEGIN

struct CMaterialDielectric : public CompDummy
{
	DECL_MANAGED_DENSE_COMP_DATA(CMaterialDielectric, 1)
};

class CSMaterialDielectric
{
public:
	static HEntity create()
	{
		MEngine& engine = MEngine::getInstance();
		HEntity h = engine.createEntity();

		CTextureBindings texs;
		texs.resize(0);

		engine.addComponent<CMaterialDielectric>(h);
		engine.addComponent<CTextureBindings>(h, std::move(texs));
		engine.addComponent<CBSDFRequests>(h);
		return h;
	}
};

class JobGenerateBSDFMaterialDielectric : public JobParallazible
{
	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override;
};


WPBR_END
