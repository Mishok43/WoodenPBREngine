#pragma once

#include "pch.h"
#include "CLight.h"
#include "CSSphere.h"
#include "CTextureBase.h"
#include "WoodenMathLibrarry/DDistribution.h"

WPBR_BEGIN


struct CPosition
{
	DPoint3f p;

	DECL_MANAGED_DENSE_COMP_DATA(CPosition, 1)
};

struct CLMapDistribution
{
	DDistributionbPieceWise2D<float>  d;

	DECL_MANAGED_DENSE_COMP_DATA(CLMapDistribution, 1)
};


class SLightInfiniteArea
{
public:
	static HEntity create(Spectrum lemit, const std::string& lightMap, const DTransformf& world);
};

class JobLightInfiniteAreaPreprocessLocation: public JobParallaziblePerCompGroup<CPosition, CSphere>
{
public:
	void update(WECS* ecs, HEntity hEntity, CPosition& pos, CSphere& sphere) override;
};

class JobLightInfiniteAreaPreprocessSampling : public JobParallaziblePerCompGroup<CLight, CTextureBinding2DRGB>
{
public:
	void update(WECS* ecs, HEntity hEntity, CLight& light, CTextureBinding2DRGB& lMap) override;
};

class JobLightInfiniteAreaLeCompute: public JobParallaziblePerCompGroup<CLight,  CLightLeComputeRequests,CTextureBinding2DRGB, CTransform>
{
public:
	void update(WECS* ecs, HEntity hEntity, CLight& l, CLightLeComputeRequests& requests, CTextureBinding2DRGB& lMap, CTransform& world) override;

};

class JobLightInfiniteAreaLiSample : public JobParallaziblePerCompGroup<CLight, CLightLiSampleRequests, CLMapDistribution, CTextureBinding2DRGB, CTransform>
{
public:
	void update(WECS* ecs, HEntity hEntity, CLight& l, CLightLiSampleRequests& requests, CLMapDistribution& distr, CTextureBinding2DRGB& lMap, CTransform& world) override;

};

class JobLightInfiniteAreaLiCompute : public JobParallaziblePerCompGroup<CLight, CLightLiComputeRequests, CLMapDistribution, CTextureBinding2DRGB, CTransform>
{
public:
	void update(WECS* ecs, HEntity hEntity, CLight& l, CLightLiComputeRequests& requests, CLMapDistribution& distr, CTextureBinding2DRGB& lMap, CTransform& world) override;

};

WPBR_END



