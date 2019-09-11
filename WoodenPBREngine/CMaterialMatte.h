#pragma once

#include "pch.h"
#include "CBXDF.h"
#include "CMaterial.h"
#include "CSpectrum.h"

WPBR_BEGIN


void computeScattering(
	HEntity hEntity,
	const CTextureDiffuse& kd,
	const CTextureRoughness& sigma,
	const CTextureBumpMap& bumpMap,
	const CSurfInteraction& si,
	TransportMode mode,
	bool allowMultipleLobes
)
{
	Spectrum diffuse = kd.sample(si);
	float sigma = clamp(kd.sample(si), 0, 90);
	if (!diffuse.isBlack())
	{
		CBXDFOreanNayar oreanNayar = CBXDFOreanNayar(sigma);
		ecs.addComponent<CBXDFOreanNayar>(hEntity, oreanNayar);
	}

}

WPBR_END
