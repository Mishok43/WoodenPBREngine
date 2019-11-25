#pragma once
#include "pch.h"
#include "CPhaseFunction.h"

WPBR_BEGIN

struct CPhaseFunctionHG
{
	float g;

	DECL_MANAGED_DENSE_COMP_DATA(CPhaseFunctionHG, 16)
};

class SPhaseFunctionHG
{
public:
	static float p(const CPhaseFunctionHG& hg, const DVector3f& wo, const DVector3f& wi);

	static float p(const CPhaseFunctionHG& hg, const float cosTheta);
};

class JobPhaseFunctionHDCompute: public JobParallaziblePerCompGroup<CPhaseFunctionHG, CPhaseFunctionRequests>
{
	void update(WECS* ecs, HEntity hEntity, CPhaseFunctionHG& hg, CPhaseFunctionRequests& requests) override;
};

WPBR_END

