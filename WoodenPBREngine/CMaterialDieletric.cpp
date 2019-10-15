#include "pch.h"
#include "CMaterialDielectric.h"

WPBR_BEGIN
DECL_OUT_COMP_DATA(CMaterialDielectric)
void JobGenerateBSDFMaterialDielectric::update(WECS* ecs, uint8_t iThread)
{

	uint32_t nRequests = queryComponentsGroup<CMaterialDielectric, CBSDFRequests>().size();
	uint32_t sliceSize = (nRequests + getNumThreads() - 1) / getNumThreads();

	ComponentsGroupSlice<CMaterialDielectric, CTextureBindings, CBSDFRequests> requests =
		queryComponentsGroupSlice<CMaterialDielectric, CTextureBindings, CBSDFRequests>(Slice(iThread * sliceSize, sliceSize));

	for_each([&](HEntity hEntity, const CMaterialDielectric& material, const CTextureBindings& texs, CBSDFRequests& requests)
	{
		for (uint32_t i = 0; i < requests.data.size(); i++)
		{
			HEntity h = requests.data[i];
			const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(h);
			const CTextureMappedPoint& mp = ecs->getComponent<CTextureMappedPoint>(h);

			float sigmaV = 0.75f;
			CReflectDirSamplerMicroface microfaceDistr;
			microfaceDistr.alphax = CReflectDirSamplerMicroface::roughnessToAlpha(sigmaV);
			microfaceDistr.alphay = CReflectDirSamplerMicroface::roughnessToAlpha(sigmaV);

			CSpectrumScale R = Spectrum(1.0);
			CFresnelDielectric Dielectric;
			Dielectric.etaI = 1.0f;
			Dielectric.etaT = 1.5f;

			HEntity hBSDF = ecs->createEntity();
			ecs->addComponent<CReflectDirSamplerMicroface>(hBSDF, std::move(microfaceDistr));
			ecs->addComponent<CSpectrumScale>(hBSDF, std::move(R));
			ecs->addComponent<CFresnelDielectric>(hBSDF, std::move(Dielectric));
			ecs->addComponent<CSampledBSDF>(h, hBSDF);
		}
		requests.data.clear();
	}, requests);
}
WPBR_END

