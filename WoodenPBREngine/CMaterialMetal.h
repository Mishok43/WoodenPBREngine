#pragma once

#include "pch.h"
#include "CBXDF.h"
#include "CMaterial.h"
#include "CSpectrum.h"
#include "CTexture.h"
#include "WoodenMathLibrarry/Utils.h"
#include "CBSDFConductorMicroface.h"
#include "SScattering.h"
WPBR_BEGIN

struct CMaterialMetal: public CompDummy
{
	DECL_MANAGED_DENSE_COMP_DATA(CMaterialMetal, 1)
}; 

class CSMaterialMetal
{
public:
	static HEntity create(const std::string& texDiffFile, const std::string& texRoughnessFile)
	{
		MEngine& engine = MEngine::getInstance();
		HEntity h = engine.createEntity();
			
		CTextureBindings texs;
		texs.resize(2);
		texs[0] = STextureBindingRGB::create(texDiffFile);
		texs[1] = STextureBindingRGB::create(texRoughnessFile);

		engine.addComponent<CMaterialMetal>(h);
		engine.addComponent<CTextureBindings>(h, std::move(texs));
		engine.addComponent<CBSDFRequests>(h);
		return h;
	}
};

class JobGenerateBSDFMaterialMetal : public JobParallazible
{
	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override;
};


WPBR_END
