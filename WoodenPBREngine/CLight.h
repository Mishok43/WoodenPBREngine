#pragma once

#include "pch.h"
#include "CSufraceInteraction.h"
#include "CSpectrum.h"
#include "CTransform.h"
#include "WoodenMathLibrarry/DPoint.h"

WPBR_BEGIN

struct CLightLiSampleRequests
{
	std::vector<HEntity, AllocatorAligned2<HEntity>> data;

	DECL_MANAGED_DENSE_COMP_DATA(CLightLiSampleRequests, 16)
}; 

struct CLightLiComputeRequests
{
	std::vector<HEntity, AllocatorAligned2<HEntity>> data;
	DECL_MANAGED_DENSE_COMP_DATA(CLightLiComputeRequests, 16)
};

struct CLightLeComputeRequests
{
	std::vector<HEntity, AllocatorAligned2<HEntity>> data;
	DECL_MANAGED_DENSE_COMP_DATA(CLightLeComputeRequests, 16)
};

struct CLightLE
{
	Spectrum L;
	DECL_MANAGED_DENSE_COMP_DATA(CLightLE, 16)
};

struct CLight
{
	Spectrum LEmit;
	DECL_MANAGED_DENSE_COMP_DATA(CLight, 16)
}; 


struct CLightPosition : public DPoint3f
{
	DECL_MANAGED_DENSE_COMP_DATA(CLightPosition, 16)
}; 


struct CLightPoint: public CompDummy
{
	DECL_MANAGED_DENSE_COMP_DATA(CLightPoint, 16)
};

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
