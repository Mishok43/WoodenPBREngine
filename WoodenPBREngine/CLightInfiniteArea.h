#pragma once

#include "pch.h"
#include "CLight.h"
#include "CSSphere.h"
#include "CTexture.h"
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
	static HEntity create(Spectrum lemit, const std::string& lightMap);
};

class JobLightInfiniteAreaPreprocessLocation: public JobParallaziblePerCompGroup<CPosition, CSphere>
{
public:
	void update(WECS* ecs, HEntity hEntity, CPosition& pos, CSphere& sphere) override;
};

class JobLightInfiniteAreaPreprocessSampling : public JobParallaziblePerCompGroup<CLight, CTextureBindingRGB>
{
public:
	void update(WECS* ecs, HEntity hEntity, CLight& light, CTextureBindingRGB& lMap) override;
};

class JobLightInfiniteAreaLeCompute: public JobParallaziblePerCompGroup<CLight,  CLightLeComputeRequests,CTextureBindingRGB>
{
public:
	void update(WECS* ecs, HEntity hEntity, CLight& l, CLightLeComputeRequests& requests, CTextureBindingRGB& lMap) override;

};

class JobLightInfiniteAreaLiSample : public JobParallaziblePerCompGroup<CLight, CLightLiSampleRequests, CLMapDistribution, CTextureBindingRGB>
{
public:
	void update(WECS* ecs, HEntity hEntity, CLight& l, CLightLiSampleRequests& requests, CLMapDistribution& distr, CTextureBindingRGB& lMap) override;

};

class JobLightInfiniteAreaLiCompute : public JobParallaziblePerCompGroup<CLight, CLightLiComputeRequests, CLMapDistribution, CTextureBindingRGB>
{
public:
	void update(WECS* ecs, HEntity hEntity, CLight& l, CLightLiComputeRequests& requests, CLMapDistribution& distr, CTextureBindingRGB& lMap) override;

};

WPBR_END



