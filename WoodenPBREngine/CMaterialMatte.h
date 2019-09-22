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

struct CMaterialMatte: public CompDummy
{
	DECL_MANAGED_DENSE_COMP_DATA(CMaterialMatte, 1)
}; DECL_OUT_COMP_DATA(CMaterialMatte)

class CSMaterialMatte
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

		engine.addComponent<CTextureBindings>(h, std::move(texs));
		engine.addComponent<CMaterialMatte>(h);
		engine.addComponent<CBSDFRequests>(h);

		return h;
	}

};

class JobGenerateBSDFMaterialMatte : public JobParallazible
{
	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override
	{

		uint32_t nRequests = queryComponentsGroup<CMaterialMatte, CBSDFRequests>().size();
		uint32_t sliceSize = (nRequests + getNumThreads()-1) /getNumThreads();

		ComponentsGroupSlice<CMaterialMatte, CTextureBindings, CBSDFRequests> requests =
			queryComponentsGroupSlice<CMaterialMatte, CTextureBindings, CBSDFRequests>(Slice(iThread * sliceSize, sliceSize));

		for_each([&](HEntity hEntity, const CMaterialMatte& material, const CTextureBindings& texs, CBSDFRequests& requests)
		{
			for (uint32_t i = 0; i < requests.data.size(); i++)
			{
				HEntity h = requests.data[i];
				const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(h);
				const CTextureMappedPoint& mp = ecs->getComponent<CTextureMappedPoint>(h);

				const CTextureBindingRGB& kd = ecs->getComponent<CTextureBindingRGB>(texs[0]);
				const CTextureBindingRGB& sigma = ecs->getComponent<CTextureBindingRGB>(texs[1]);

				CTextureSamplerAnistropic anistropic16x;
				anistropic16x.maxAnisotropy = 16;
				CFilterTableGaussing filterTable = *ecs->getComponentsRawData<CFilterTableGaussing>();

				RGBSpectrum diffuse = STextureSampleAnisotropic::sample(mp, anistropic16x, filterTable, kd.getTex(ecs));
				RGBSpectrum roughness = STextureSamplerIsotropic::sample(mp, sigma.getTex(ecs));
				float s = roughness.x();
				float sigmaV = wml::clamp(s, 0, 90);

				CReflectDirSamplerMicroface microfaceDistr;
				microfaceDistr.alphax = CReflectDirSamplerMicroface::roughnessToAlpha(sigmaV);
				microfaceDistr.alphay = CReflectDirSamplerMicroface::roughnessToAlpha(sigmaV);

				CSpectrumScale R = Spectrum(1.0f);
				CFresnelConductor conductor;
				conductor.k = Spectrum(RGBSpectrum(1.0f) - diffuse);
				conductor.etaI = Spectrum(1.0f);
				conductor.etaT = Spectrum(1.5f);

				HEntity hBSDF = ecs->createEntity();
				ecs->addComponent<CReflectDirSamplerMicroface>(hBSDF, std::move(microfaceDistr));
				ecs->addComponent<CSpectrumScale>(hBSDF, std::move(R));
				ecs->addComponent<CFresnelConductor>(hBSDF, std::move(conductor));
				ecs->addComponent<CSampledBSDF>(hEntity, hBSDF);
			}
			requests.data.clear();
		}, requests);
	}
};


WPBR_END
