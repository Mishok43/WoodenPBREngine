#include "pch.h"
#include "CMaterialMetal.h"

WPBR_BEGIN
DECL_OUT_COMP_DATA(CMaterialMetal)
void JobGenerateBSDFMaterialMetal::update(WECS* ecs, uint8_t iThread)
{

	uint32_t nRequests = queryComponentsGroup<CMaterialMetal, CBSDFRequests>().size();
	uint32_t sliceSize = (nRequests + getNumThreads() - 1) / getNumThreads();

	ComponentsGroupSlice<CMaterialMetal, CTextureBindings, CBSDFRequests> requests =
		queryComponentsGroupSlice<CMaterialMetal, CTextureBindings, CBSDFRequests>(Slice(iThread * sliceSize, sliceSize));

	for_each([&](HEntity hEntity, const CMaterialMetal& material, const CTextureBindings& texs, CBSDFRequests& requests)
	{
		for (uint32_t i = 0; i < requests.data.size(); i++)
		{
			HEntity h = requests.data[i];
			const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(h);
			const CTextureMappedPoint& mp = ecs->getComponent<CTextureMappedPoint>(h);

			const CTextureBindingRGB& kd = ecs->getComponent<CTextureBindingRGB>(texs[0]);
			const CTextureBindingRGB& sigma = ecs->getComponent<CTextureBindingRGB>(texs[1]);

			CTextureSamplerAnistropic anistropic16x;
			anistropic16x.maxAnisotropy = 32;
			CFilterTableGaussing filterTable = *ecs->getComponentsRawData<CFilterTableGaussing>();

			RGBSpectrum metallic = STextureSampleAnisotropic::evaluate(mp, anistropic16x, filterTable, kd.getTex(ecs));
			RGBSpectrum roughness = STextureSampleAnisotropic::evaluate(mp, anistropic16x, filterTable, sigma.getTex(ecs));

			//RGBSpectrum metallic = STextureSamplerIsotropic::sample(mp, kd.getTex(ecs));
			//RGBSpectrum roughness = STextureSamplerIsotropic::sample(mp, sigma.getTex(ecs));
		
			float s = roughness.x();
			float sigmaV = wml::clamp(s*0.45, 0, 90);

			CReflectDirSamplerMicroface microfaceDistr;
			microfaceDistr.alphax = CReflectDirSamplerMicroface::roughnessToAlpha(sigmaV);
			microfaceDistr.alphay = CReflectDirSamplerMicroface::roughnessToAlpha(sigmaV);

			CSpectrumScale R = Spectrum(1.0);
			CFresnelConductor conductor;
			conductor.k = metallic;
			conductor.etaI = Spectrum(1.0f);
			conductor.etaT = Spectrum(1.5f);

			HEntity hBSDF = ecs->createEntity();
			ecs->addComponent<CReflectDirSamplerMicroface>(hBSDF, std::move(microfaceDistr));
			ecs->addComponent<CSpectrumScale>(hBSDF, std::move(R));
			ecs->addComponent<CFresnelConductor>(hBSDF, std::move(conductor));
			ecs->addComponent<CSampledBSDF>(h, hBSDF);
		}
		requests.data.clear();
	}, requests);
}
WPBR_END

