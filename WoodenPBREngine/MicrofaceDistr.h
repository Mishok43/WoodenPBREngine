#pragma once
#include "pch.h"
#include "WoodenMathLibrarry/DVector.h"
#include "WoodenMathLibrarry/Utils.h"
WPBR_BEGIN

struct CMicrofaceDistrTrowbridgeReitz
{
	const float alphax, alphay;

	static inline float roughnessToAlpha(float roughness);

	float evaluate(const DVector3f& wh) const
	{
		float t2Theta = tan2Theta(wh);
		if (std::isinf(t2Theta))
		{
			return 0.;
		}

		const float cos4Theta = cos2Theta(wh) * cos2Theta(wh);
		float e = (cos2Phi(wh) / (alphax * alphax) +
				   sin2Phi(wh) / (alphay * alphay)) * t2Theta;
		return 1 / (PI * alphax * alphay * cos4Theta * (1 + e) * (1 + e));
	}
};

WPBR_END