#pragma once
#include "pch.h"
#include "CMedium.h"

WPBR_BEGIN


struct CMediumHomogeneousTransmittance
{
	Spectrum tr;
};


struct CMediumHomogeneous
{
	Spectrum albedo;
	Spectrum scater;

	DECL_MANAGED_DENSE_COMP_DATA(CMediumHomogeneous, 4)
};

class SMediumHomogeneous
{
public:
	static HEntity create(Spectrum sigmaA, Spectrum sigmaS, Spectrum sigmaT, float g);

	static Spectrum transmittance(const DRayf& ray, float tMax, const Spectrum& sigmaT);
};


class JobMediumHomogeneousTransmitance : public JobParallaziblePerCompGroup<CMediumHomogeneousTransmittance, CMediumTransmittanceRequests>
{
	void update(WECS* ecs, HEntity hEntity, CMediumHomogeneousTransmittance& transmittance, CMediumTransmittanceRequests& requests) override;
};


WPBR_END





