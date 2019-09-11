#pragma once

#include "pch.h"
#include "CBXDF.h"
#include "CMaterial.h"
#include "CSpectrum.h"

WPBR_BEGIN


void computeScattering(
	HEntity hEntity,
	const CTextureDiffuse& kd,
	const CTextureSpecular& ks,
	const CTextureRoughness& roughness,
	const CTextureBumpMap& bumpMap,
	const CSurfInteraction& si,
	TransportMode mode,
	bool allowMultipleLobes
)
{


	Spectrum ks = ks.sample(si);
	if (!ks.isBlack())
	{
		float rough = roughness.sample(si);

		CMicrofaceDistrTrowbridgeReitz microfaceDistr;
		microfaceDistr.alphax = microfaceDistr.alphay = rough;

		ecs.addComponent<CMicrofaceDistrTrowbridgeReitz>(hEntity, std::move(CMicrofaceDistrTrowbridgeReitz));

		CFresnelDielectric fresnelDielectric;
		fresnelDielectric.etaI = 1.0f;
		fresnelDielectric.etaT = 1.5f;

		ecs.addComponent<CFresnelDielectric>(hEntity, std::move(CMicrofaceDistrTrowbridgeReitz));

	}

	

	

}

WPBR_END


