#pragma once
#include "pch.h"
#include "CBounds.h"
#include "CRay.h"
#include "CSufraceInteraction.h"
#include "SShape.h"
#include "MEngine.h"
#include "WoodenMathLibrarry/HSolver.h"

WPBR_BEGIN

struct CSphere
{
	CSphere(float radius):
		radius(radius)
	{ }

	float radius;
	
	DECL_MANAGED_DENSE_COMP_DATA(CSphere, 16)
}; DECL_OUT_COMP_DATA(CSphere)


class SSphere: public SShape
{
	using cmp_type_list = typename wecs::type_list<CShape, CTransform, CSphere>;
public:

	static uint32_t create(CShape shape,
						   CTransform transform,
						   CSphere sphere);

	static float getArea(const CSphere& eSphere);

	static bool intersect(const CTransform& eWorld,
						  const CSphere& eSphere,
						  const DRayf& ray);

	static bool intersect(const CTransform& eWorld,
						  const CSphere& eSphere,
						  HCompR<CShape> hShape,
						  const DRayf& rayW, float& tHit,
						  CInteractionSurface& surfInter);

	static DBounds3f getBoundLocal(const CSphere& eSphere);

	static DBounds3f getBoundWorld(const CSphere& eSphere,
								   const CTransform& eWorld);

public:
	
	float getArea(const HEntity e) override
	{
		return getArea(e.getComponent<CSphere>());
	}

	bool intersect(const HEntity e,
				   const DRayf& ray) override
	{
		return intersect(e.getComponent<CTransform>(),
						 e.getComponent<CSphere>(),
						 ray);
	}

	bool intersect(const HEntity e, 
				   const DRayf& ray, float& tHit, CInteractionSurface& surfInter) override
	{
		return intersect(e.getComponent<CTransform>(),
						 e.getComponent<CSphere>(),
						 e.getComponentHandleR<CShape>(),
						 ray, tHit, surfInter);
	}

	DBounds3f getBoundLocal(const HEntity e) override
	{
		return getBoundLocal(e.getComponent<CSphere>());
	}

	DBounds3f getBoundWorld(const HEntity e) override
	{
		return getBoundWorld(e.getComponent<CSphere>(),
							 e.getComponent<CTransform>());
	}
	
};


WPBR_END


