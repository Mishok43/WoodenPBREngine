#pragma once

#include "pch.h"
#include "CSpectrum.h"
#include "WoodenMathLibrarry/DRay.h"

WPBR_BEGIN




struct CMediumTransmittanceRequest
{
	DRayf* ray;
	float tMax;

	DECL_MANAGED_DENSE_COMP_DATA(CMediumTransmittanceRequest, 4)
};

struct CMediumTransmittanceResponse
{
	Spectrum tr;

	DECL_MANAGED_DENSE_COMP_DATA(CMediumTransmittanceResponse, 4)
};


struct CMediumTransmittanceRequests
{
	std::vector<HEntity> data;

	DECL_MANAGED_DENSE_COMP_DATA(CMediumTransmittanceRequests, 4)
};

WPBR_END