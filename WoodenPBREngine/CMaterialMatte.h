#pragma once

#include "pch.h"
#include "CBXDF.h"
#include "CMaterial.h"
#include "CSpectrum.h"
#include "CTexture.h"
#include "WoodenMathLibrarry/Utils.h"
#include "CBSDFConductorMicroface.h"
WPBR_BEGIN


void computeScattering(
	WECS* ecs,
	HEntity hEntity,
	const CTextureMappedPoint& mp,
	const CTextureDiffuse& kd,
	const CTextureRoughness& sigma,
	const CTextureBumpMap& bumpMap,
	const CSurfaceInteraction& si,
	bool allowMultipleLobes
)
{
	CTextureSamplerAnistropic anistropic16x;
	anistropic16x.maxAnisotropy = 16;
	CFilterTableGaussing filterTable = *ecs->getComponentsRawData<CFilterTableGaussing>();

	RGBSpectrum diffuse = STextureSampleAnisotropic::sample(mp, anistropic16x, filterTable, kd.getTex(ecs));
	RGBSpectrum roughness = STextureSamplerIsotropic::sample(mp, sigma.getTex(ecs));
	float s = roughness.x();
	float sigma = wml::clamp(s, 0, 90);
	
	CReflectDirSamplerMicroface microfaceDistr;
	microfaceDistr.alphax = CReflectDirSamplerMicroface::roughnessToAlpha(s);
	microfaceDistr.alphay = CReflectDirSamplerMicroface::roughnessToAlpha(s);

	CSpectrumScale R = Spectrum(1.0f);
	CFresnelConductor conductor;
	conductor.k = Spectrum(RGBSpectrum(1.0f)-diffuse);
	conductor.etaI = Spectrum(1.0f);
	conductor.etaT = Spectrum(1.5f);

	ecs->addComponent<CReflectDirSamplerMicroface>(hEntity, std::move(microfaceDistr));
	ecs->addComponent<CSpectrumScale>(hEntity, std::move(R));
	ecs->addComponent<CFresnelConductor>(hEntity, std::move(conductor));
}

WPBR_END
