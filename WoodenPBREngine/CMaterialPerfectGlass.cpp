#include "pch.h"
#include "CMaterialPerfectGlass.h"

WPBR_BEGIN
DECL_OUT_COMP_DATA(CMaterialPerfectGlass)

void JobGenerateBSDFMaterialPerfectGlass::update(WECS* ecs, uint8_t iThread)
{
	uint32_t nRequests = queryComponentsGroup<CMaterialPerfectGlass, CBSDFRequests>().size();
	uint32_t sliceSize = (nRequests + getNumThreads() - 1) / getNumThreads();

	ComponentsGroupSlice<CMaterialPerfectGlass, CTextureBindings, CBSDFRequests> requests =
		queryComponentsGroupSlice<CMaterialPerfectGlass, CTextureBindings, CBSDFRequests>(Slice(iThread * sliceSize, sliceSize));

	for_each([&](HEntity hEntity, const CMaterialPerfectGlass& material, const CTextureBindings& texs, CBSDFRequests& requests)
	{
		for (uint32_t i = 0; i < requests.data.size(); i++)
		{
			HEntity h = requests.data[i];
			const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(h);
			const CTextureMappedPoint& mp = ecs->getComponent<CTextureMappedPoint>(h);

			/*const CTextureBindingRGB& kd = ecs->getComponent<CTextureBindingRGB>(texs[0]);
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
*/
			CSpectrumScale R = Spectrum(1.0f);
			CFresnelDielectric fresnel;
			fresnel.etaI = 1.0f;
			fresnel.etaT = 1.66f;
			

			HEntity hBSDF = ecs->createEntity();
			ecs->addComponent<CBXDFSpecularReflection>(hBSDF);
			ecs->addComponent<CSpectrumScale>(hBSDF, std::move(R));
			ecs->addComponent<CFresnelDielectric>(hBSDF, std::move(fresnel));
			ecs->addComponent<CSampledBSDF>(h, hBSDF);
		}
		requests.data.clear();
	}, requests);
}
WPBR_END
