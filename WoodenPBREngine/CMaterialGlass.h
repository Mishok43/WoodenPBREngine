#pragma once

#include "pch.h"
#include "CBXDF.h"
#include "CMaterial.h"
#include "CSpectrum.h"
#include "CTextureBase.h"
#include "WoodenMathLibrarry/Utils.h"
#include "MicrofaceDistr.h"
#include "SScattering.h"
WPBR_BEGIN
//
//struct CMaterialGlass : public CompDummy
//{
//	DECL_MANAGED_DENSE_COMP_DATA(CMaterialGlass, 1)
//};
//
//class SMaterialGlass
//{
//public:
//	static HEntity create()
//	{
//		MEngine& engine = MEngine::getInstance();
//		HEntity h = engine.createEntity();
//
//		CTextureBindings texs;
//		texs.resize(0);
//
//		engine.addComponent<CMaterialGlass>(h);
//		engine.addComponent<CTextureBindings>(h, std::move(texs));
//		engine.addComponent<CBSDFRequests>(h);
//		return h;
//	}
//};
//
//class JobGenerateBSDFMaterialGlass : public JobParallazible
//{
//	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
//	{
//		return nWorkThreads;
//	}
//
//	void update(WECS* ecs, uint8_t iThread) override;
//};


WPBR_END




