#pragma once
#include "pch.h"
#include "CSufraceInteraction.h"
#include "CTransform.h"
#include "CBounds.h"
#include "CCentroid.h"
#include "WoodenECS/Job.h"
#include "CTextureMapping.h"
#include "CSpectrum.h"
#include "WoodenMathLibrarry/Utils.h"
#include "WoodenMathLibrarry/DRay.h"
#include "WoodenMathLibrarry/DBounds.h"
#include "CLight.h"
#include "CSSampler.h"
#include "SScattering.h"

WPBR_BEGIN



struct CInteractionSphere
{
	DRayf rayL;
	DPoint3f pHitL;

	DECL_MANAGED_DENSE_COMP_DATA(CInteractionSphere, 16)
};

struct CSphere
{
	CSphere()
	{
	};

	CSphere(float radius):
		radius(radius)
	{ }

	float radius;
	
	DECL_MANAGED_DENSE_COMP_DATA(CSphere, 16)
}; 


namespace SSphere
	{
		uint32_t create(CTransform transform,
						CSphere sphere);


		float getArea(const CSphere& eSphere);

		float pdf(
			const CSphere& sphere,
			const CTransform& transform,
			const CInteraction& inter,
			const DVector3f& wi);

		CTextureMappedPoint mapUV(
			const CSphere& sphere,
			const CTransform& world,
			const CSurfaceInteraction& si
		);

		DPoint3f sample(
			const CSphere& sphere,
			const CTransform& transform,
			const CInteraction& interac,
			const DPoint2f& u,
			float& p
		);

		bool intersect(
			const DRayf& rayW,
			const CSphere& sphere,
			const CTransform& world,
			CInteractionSphere& interactionSphere,
			float& tHit);

		CSurfaceInteraction computeSurfInteraction(
			HEntity hSphere,
			const CSphere& sphere,
			const CTransform& world,
			const CInteractionSphere& interactionSphere);

		/*bool intersect(const CTransform& eWorld,
							  const CSphere& eSphere,
							  const DRayf& rayW, float& tHit,
							  CInteractionSurface& surfInter);*/

		DBounds3f getBoundLocal(const CSphere& eSphere);

		DBounds3f getBoundWorld(const CSphere& eSphere,
								const CTransform& eWorld);

	};


class JobProcessSphereFullInteractionRequests: public JobParallazible
{
	constexpr static uint32_t slice = 64;

	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return 1;
	}

	void update(WECS* ecs, uint8_t iThread) override;

	void finish(WECS* ecs) override;
};

class JobProcessSphereSurfInteractionRequests: public JobParallazible
{
	constexpr static uint32_t slice = 64;

	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return 1;
	}

	void update(WECS* ecs, uint8_t iThread) override;
};


class JobUpdateBoundsAndCentroidSphere : public JobParallazible
{
	constexpr static uint32_t slice = 64;

	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		ComponentsGroup<CSphere> collisions = queryComponentsGroup<CSphere>();
		return min(nWorkThreads, collisions.size<CSphere>() / slice);
	}

	void update(WECS* ecs, uint8_t iThread) override
	{

		uint32_t nCollisions = queryComponentsGroup<CSphere>().size<CSphere>();
		uint32_t sliceSize = (nCollisions + getNumThreads()-1) /getNumThreads();
		uint32_t iStart = iThread * sliceSize;

		ComponentsGroupSlice<CSphere, CTransform, CBounds, CCentroid> spheres =
			queryComponentsGroupSlice<CSphere, CTransform, CBounds, CCentroid>(Slice(iStart, sliceSize));

		for_each([](HEntity hEntity, const CSphere& eSphere, const CTransform& eTransform,
				 CBounds& eBounds, CCentroid& eCentroid)
		{
			eBounds = SSphere::getBoundWorld(eSphere, eTransform);
			eCentroid = CCentroid(eBounds);
		}, spheres);
	}
};


class JobSphereProcessMapUVRequests: public JobParallazible
{
	constexpr static uint32_t slice = 64;

	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return min(nWorkThreads, (queryComponentsGroup<CSphere, CMapUVRequests>().size()+slice-1)/ slice);
	}

	void update(WECS* ecs, uint8_t iThread) override
	{

		uint32_t nRequests = queryComponentsGroup<CSphere>().size<CSphere>();
		uint32_t sliceSize = (nRequests + getNumThreads()-1) /getNumThreads();

		ComponentsGroupSlice<CSphere, CTransform,  CMapUVRequests> spheres =
			queryComponentsGroupSlice<CSphere, CTransform, CMapUVRequests>(Slice(iThread * sliceSize, sliceSize));

		for_each([&](HEntity hEntity, const CSphere& sphere, const CTransform& world, CMapUVRequests& requests)
		{
			for (uint32_t i = 0; i < requests.si.size(); i++)
			{
				const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(requests.si[i]);
				CTextureMappedPoint mp = SSphere::mapUV(sphere, world, si);
				ecs->addComponent<CTextureMappedPoint>(requests.si[i], mp);
			}
			requests.si.clear();
		}, spheres);
	}
};

class JobSphereLightProcessSamplingRequests : public JobParallazible
{
	constexpr static uint32_t slice = 64;

	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return min(nWorkThreads, (queryComponentsGroup<CSphere, CLightLiSampleRequests>().size() + slice - 1) / slice);
	}

	void update(WECS* ecs, uint8_t iThread) override
	{

		uint32_t nRequests = queryComponentsGroup<CSphere>().size<CSphere>();
		uint32_t sliceSize = (nRequests + getNumThreads()-1) /getNumThreads();

		ComponentsGroupSlice<CSphere, CTransform, CLightLiSampleRequests, CLight> spheres =
			queryComponentsGroupSlice<CSphere, CTransform, CLightLiSampleRequests, CLight>(Slice(iThread * sliceSize, sliceSize));

		for_each([&](HEntity hEntity, const CSphere& sphere, const CTransform& world, CLightLiSampleRequests& requests, const CLight& light)
		{
			for (uint32_t i = 0; i < requests.data.size(); i++)
			{
				HEntity hRequest = requests.data[i];
				const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(requests.data[i]);
				CSamples2D& samples = ecs->getComponent<CSamples2D>(requests.data[i]);

				float pdfWi;

				CSampledLightPDF pdf;
				CSampledLightLI li;

				DPoint3f sP = SSphere::sample(sphere, world, si, samples.next(), pdfWi);			
				li = light.LEmit;
				pdf.p = pdfWi;

				CSampledWI wi = normalize(sP - si.p);
				ecs->addComponent<CSampledWI>(hRequest, std::move(wi));
				ecs->addComponent<CSampledLightPDF>(hRequest, std::move(pdf));
				ecs->addComponent<CSampledLightLI>(hRequest, std::move(li));
			}
			requests.data.clear();
		}, spheres);
	}
};

class JobSphereLightProcessComputeRequests : public JobParallazible
{
	constexpr static uint32_t slice = 64;

	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return min(nWorkThreads, (queryComponentsGroup<CSphere, CLightLiComputeRequests>().size() + slice - 1) / slice);
	}

	void update(WECS* ecs, uint8_t iThread) override
	{

		uint32_t nRequests = queryComponentsGroup<CSphere>().size<CSphere>();
		uint32_t sliceSize = (nRequests + getNumThreads()-1) /getNumThreads();

		ComponentsGroupSlice<CSphere, CTransform, CLightLiComputeRequests, CLight> spheres =
			queryComponentsGroupSlice<CSphere, CTransform, CLightLiComputeRequests, CLight>(Slice(iThread * sliceSize, sliceSize));

		for_each([&](HEntity hEntity, const CSphere& sphere, const CTransform& world, CLightLiComputeRequests& requests, const CLight& light)
		{
			for (uint32_t i = 0; i < requests.data.size(); i++)
			{
				HEntity hRequest = requests.data[i];
				const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(hRequest);
				const CSampledWI& wi = ecs->getComponent<CSampledWI>(hRequest);
			
				CSampledLightPDF pdfWI;
				pdfWI.p = SSphere::pdf(sphere, world, si, wi);

				ecs->addComponent<CSampledLightPDF>(hRequest, std::move(pdfWI));
				ecs->addComponent<CSampledLightLI>(hRequest, light.LEmit);
			}
			requests.data.clear();
		}, spheres);
	}
};





WPBR_END


