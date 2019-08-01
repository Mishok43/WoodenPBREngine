#pragma once
#include "pch.h"
#include "CBounds.h"
#include "CRay.h"
#include "CSufraceInteraction.h"
#include "MEngine.h"
#include "WoodenMathLibrarry/HSolver.h"

WPBR_BEGIN

struct CSphere
{
	float radius;
	float zMin, zMax;
	float thetaMin, thetaMax, phiMax;

	DECL_MANAGED_DENSE_COMP_DATA(CSphere, 16)
}; DECL_OUT_COMP_DATA(CSphere)


class SShapeSphere
{
	using cmp_type_list = typename wecs::type_list<CShape, CSphere, CTransform>;

	struct Entity
	{
		CShape& shape;
		CSphere& sphere;
		CTransform& world;
	};

public:
	static uint32_t createSphere(CShape shape,
								 CSphere sphere,
								 CTransform transform)
	{
		MEngine& engine = MEngine::getInstance();
		uint32_t hEntity = engine.createEntity();
		engine.addComponent<CTransform>(hEntity, std::move(transform));
		engine.addComponent<CShape>(hEntity, std::move(shape));
		engine.addComponent<CSphere>(hEntity, std::move(sphere));

		return hEntity;
	}

	float getArea()
	{
	}

	static bool Intersect(const Entity& e,
						  const CRay& ray)
	{
		float tHit = ray.tMax;
		CInteractionSurface surfInter;
		return Intersect(e, ray, tHit, surfInter);
	}

	static bool Intersect(const Entity& e,
						  const CRay& ray, float& tHit,
						  CInteractionSurface& surfInter)
	{

		
		float phi;
		DPoint3f pHit;


		DVector3f oErr, dErr;
		DRay ray = e.world(ray, INV_TRANFORM);

		float a = ray.dir.length2();
		float b = 2 * (dot(ray.dir, ray.origin));
		float c = ray.origin.length2() - e.sphere.radius*e.sphere.radius;

		float t0, t1;
		if (!HSolver::getRootsQuadraticEquation(a, b, c, t0, t1))
		{

		}
	}


	static DBounds3f getBoundLocal(const Entity& e)
	{
		return DBounds3f(DVector3f(-e.sphere.radius, -e.sphere.radius, e.sphere.zMin),
						 DVector3f(e.sphere.radius, e.sphere.radius, e.sphere.zMax));

	}

	static DBounds3f getBoundWorld(const Entity& e)
	{
		return e.world(getBoundWorld(e));
	}
};


WPBR_END


