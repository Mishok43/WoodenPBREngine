#pragma once

#include "pch.h"
#include "CSufraceInteraction.h"
#include "CSpectrum.h"
#include "CTransform.h"
#include "WoodenMathLibrarry/DPoint.h"

WPBR_BEGIN

struct CLightSamplingRequests
{
	std::vector<HEntity, AllocatorAligned<HEntity>> data;

	DECL_MANAGED_DENSE_COMP_DATA(CLightSamplingRequests, 16)
}; DECL_OUT_COMP_DATA(CLightSamplingRequests)

struct CLightComputeRequests
{
	std::vector<HEntity, AllocatorAligned<HEntity>> data;
	DECL_MANAGED_DENSE_COMP_DATA(CLightComputeRequests, 16)
}; DECL_OUT_COMP_DATA(CLightComputeRequests)


struct CLight
{
	Spectrum LEmit;
	DECL_MANAGED_DENSE_COMP_DATA(CLight, 16)
}; DECL_OUT_COMP_DATA(CLight)



struct CLightPosition : public DPoint3f
{
	DECL_MANAGED_DENSE_COMP_DATA(CLightPosition, 16)
}; DECL_OUT_COMP_DATA(CLightPosition)


struct CLightPoint: public CompDummy
{
	DECL_MANAGED_DENSE_COMP_DATA(CLightPoint, 16)
}; DECL_OUT_COMP_DATA(CLightPoint)

class JobLightPointsSample
{
	Spectrum sample_li(
		const CLight& l,
		const CLightPoint& light,
		const CLightPosition& lp,
		const CInteraction& ref,
		DVector3f& wi,
		DPoint3f& pos,
		float& pdf) const
	{
		DVector3f dis = lp - ref.p;
		float disLength2 = dis.length2();
		wi = dis / std::sqrt(disLength2);
		pdf = 1.0f;
		return l.LEmit /disLength2;
	}
};

struct CLightSpot
{
	float cosTotal;
	float cosFalloffStart;
};

class JobLightSpotSample
{

	Spectrum sample_li(
		const CLightSpot& spot,
		const CLight& l,
		const CLightPosition& lp,
		const CTransform& lworld,
		const CInteraction& ref,
		DVector3f& wi,
		DPoint3f& pos,
		float& pdf
	)
	{
		DVector3f dis = lp - ref.p;
		float disLength2 = dis.length2();
		wi = dis / std::sqrt(disLength2);
		pdf = 1.0f;
			
		DVector3f wil = lworld(-wi, INV_TRANFORM);

		float falloff;

		float cosTheta = wil.z();
		if (cosTheta < spot.cosTotal)
		{
			falloff = 0;
		}
		if (cosTheta > spot.cosFalloffStart)
		{
			falloff = 1;
		}
		else
		{
			float delta = (cosTheta - spot.cosTotal) /
				(spot.cosFalloffStart - spot.cosTotal);
			falloff = (delta * delta) * (delta * delta);
		}
		return l.LEmit *(falloff/ disLength2);
	}
};


WPBR_END
