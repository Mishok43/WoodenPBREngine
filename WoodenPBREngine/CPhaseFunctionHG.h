
#pragma once
#include "pch.h"
#include "CPhaseFunction.h"

WPBR_BEGIN

struct CPhaseFunctionHG
{
	float g;
};

class JobPhaseFunctionHDCompute: public JobParallaziblePerCompGroup<CPhaseFunctionHG, CPhaseFunctionRequests>
{
	void update(WECS* ecs, HEntity hEntity, CPhaseFunctionHG& hg, CPhaseFunctionRequests& requests) override;
};

WPBR_END

