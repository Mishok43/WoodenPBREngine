#pragma once
#include "pch.h"
#include "CSufraceInteraction.h"
#include "CTransform.h"
#include "CBounds.h"
#include "CCentroid.h"
#include "WoodenECS/Job.h"
#include "WoodenMathLibrarry/HSolver.h"
#include "WoodenMathLibrarry/DRay.h"
#include "WoodenMathLibrarry/DBounds.h"

WPBR_BEGIN



struct CInteractionSphere
{
	DRayf rayL;
	DPoint3f pHitL;
	HEntity hSphere;

	DECL_MANAGED_DENSE_COMP_DATA(CInteractionSphere, 16)
}; DECL_OUT_COMP_DATA(CInteractionSphere)

struct CSphere
{
	CSphere(float radius):
		radius(radius)
	{ }

	float radius;
	
	DECL_MANAGED_DENSE_COMP_DATA(CSphere, 16)
}; DECL_OUT_COMP_DATA(CSphere)



class JobProcessSphereFullInteractionRequests: public JobParallazible
{
	constexpr static uint32_t slice = 64;

	void updateNStartThreads(uint8_t nWorkThreads) override
	{
		ComponentsGroup<CFullInteractionRequest> collisions = queryComponentsGroup<CFullInteractionRequest>();
		nThreads = std::min(nWorkThreads, collisions.size<CFullInteractionRequest>()/ slice);
	}

	void update(WECS* ecs, uint8_t iThread) override;
};

class JobProcessSphereInteractionRequests: public JobParallazible
{
	constexpr static uint32_t slice = 64;

	void updateNStartThreads(uint8_t nWorkThreads) override
	{
		ComponentsGroup<CInteractionSphere> collisions = queryComponentsGroup<CInteractionSphere>();
		nThreads = std::min(nWorkThreads, collisions.size<CInteractionSphere>()/ slice);
	}

	void update(WECS* ecs, uint8_t iThread) override;
};

class JobUpdateBoundsAndCentroidSphere : public JobParallazible
{
	constexpr static uint32_t slice = 64;

	void updateNStartThreads(uint8_t nWorkThreads) override
	{
		ComponentsGroup<CSphere> collisions = queryComponentsGroup<CSphere>();
		nThreads = std::min(nWorkThreads, collisions.size<CSphere>() / slice);
	}

	void update(WECS* ecs, uint8_t iThread) override
	{

		uint32_t nCollisions = queryComponentsGroup<CSphere>().size<CSphere>();
		uint32_t sliceSize = (nCollisions + nThreads - 1) / nThreads;
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



class SSphere
{
	using cmp_type_list = typename wecs::type_list<CShape, CTransform, CSphere, CBounds, CCentroid>;
public:

	static uint32_t create(CShape shape,
						   CTransform transform,
						   CSphere sphere);


	static float getArea(const CSphere& eSphere);

	/*bool intersect(const CTransform& eWorld,
						  const CSphere& eSphere,
						  const DRayf& rayW, float& tHit,
						  CInteractionSurface& surfInter);*/

	static DBounds3f getBoundLocal(const CSphere& eSphere);

	static DBounds3f getBoundWorld(const CSphere& eSphere,
								   const CTransform& eWorld);

};


WPBR_END


