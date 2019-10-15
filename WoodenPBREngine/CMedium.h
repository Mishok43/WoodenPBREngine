#pragma once

#include "pch.h"
#include "CSpectrum.h"
#include "WoodenMathLibrarry/DRay.h"

WPBR_BEGIN


struct CMediumTransmittance
{
	Spectrum sigma;
};


struct CMediumHomogeneous
{
	Spectrum sigmaA;
	Spectrum sigmaS;

	DECL_MANAGED_DENSE_COMP_DATA(CMediumHomogeneous, 4)
};


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

class SMediumHomogeneous
{
public :
	static HEntity create(Spectrum sigmaA, Spectrum sigmaS, Spectrum sigmaT, float g);

	static Spectrum transmittance(const DRayf& ray, float tMax, const Spectrum& sigmaT);
};


class JobMediumHomogeneousTransmitance : public JobParallaziblePerCompGroup<CMediumTransmittance, CMediumTransmittanceRequests>
{
	void update(WECS* ecs, HEntity hEntity, CMediumTransmittance& transmittance,CMediumTransmittanceRequests& requests) override;
};

WPBR_END