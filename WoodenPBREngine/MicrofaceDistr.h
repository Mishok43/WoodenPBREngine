#pragma once
#include "pch.h"
#include "WoodenMathLibrarry/DVector.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/Utils.h"
#include "WoodenMathLibrarry/Samplers.h"
#include <algorithm>


WPBR_BEGIN

struct CReflectDirSamplerMicroface
{
	

	 float alphax, alphay;
	
	static inline float roughnessToAlpha(float roughness)
	{
		roughness = max(roughness, 1e-3f);
		float x = std::log(roughness);
		return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x +
			0.000640711f * x * x * x * x;
	}

	float d(const DVector3f& wh) const
	{
		float t2Theta = tan2Theta(wh);
		if (std::isinf(t2Theta))
		{
			return 0.;
		}

		const float cos4Theta = cos2Theta(wh) * cos2Theta(wh);
		float e = (cos2Phi(wh) / (alphax * alphax) +
				   sin2Phi(wh) / (alphay * alphay)) * t2Theta;
		float pd = 1 / (PI * alphax * alphay * cos4Theta * (1 + e) * (1 + e));
		return pd;
	}

	float pdf(const DVector3f& wh) const
	{
		return d(wh) * absCosTheta(wh);
	}

	float pdf(const DVector3f& wh, float dWH) const
	{
		return dWH * absCosTheta(wh);
	}


	DVector3f sample_f(const DVector3f &wo,
					 const DPoint2f &u,
					 float& pdf) const
	{
		
		float cosTheta = 0, phi = (2 * PI) * u[1];
		if (alphax == alphay)
		{
			float tanTheta2 = alphax * alphax * u[0] / (1.0f - u[0]);
			cosTheta = 1 / std::sqrt(1 + tanTheta2);
		}
		else
		{
			phi = std::atan(alphay / alphax * std::tan(2 * PI * u[1] + .5f * PI));
			if (u[1] > .5f)
			{
				phi += PI;
			}

			float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
			const float alphax2 = alphax * alphax, alphay2 = alphay * alphay;
			const float alpha2 =
				1 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
			float tanTheta2 = alpha2 * u[0] / (1 - u[0]);
			cosTheta = 1 / std::sqrt(1 + tanTheta2);
		}
		float sinTheta = std::sqrt(max(0.f, 1.f - cosTheta * cosTheta));
		DVector3f wh = sphericalToCasterian(sinTheta, cosTheta, phi);
		if (!isSameHemisphere(wo, wh))
		{
			wh = -wh;
		}

		pdf = d(wh);
		return wh;
	}

	DECL_MANAGED_DENSE_COMP_DATA(CReflectDirSamplerMicroface, 1)
};

struct CReflectDirSamplerCosSin
{
	float p(const DVector3f &wo, const DVector3f &wi)
	{
		return isSameHemisphere(wo, wi) ? absCosTheta(wi) / PI : 0;
	}

	void sample(const DVector3f &wo,
				const DPoint2f &u,
				DVector3f& wi,
			     float& pdf) 
	{
		wi = sampleHemisphereCosSin(u);
		if (wo.z() < 0)
		{
			wi.z() *= -1;
		}

		pdf = p(wo, wi);
	}
};

WPBR_END