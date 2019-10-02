#pragma once
#include "pch.h"
//#pragma once
//#include "pch.h"
//#include "CBounds.h"
//#include "CRay.h"
//#include "CSufraceInteraction.h"
//#include "SShape.h"
//#include "MEngine.h"
//#include "WoodenMathLibrarry/HSolver.h"
//
//WPBR_BEGIN
//
//struct CCylinder
//{
//	CCylinder(float radius, float zMin, float zMax):
//		radius(radius),
//		zMin(zMin),
//		zMax(zMax)
//	{ }
//
//	float radius, zMin, zMax;
//	
//	DECL_MANAGED_DENSE_COMP_DATA(CCylinder, 4)
//}; DECL_OUT_COMP_DATA(CCylinder)
//
//
//class SCylinder: public SShape
//{
//	using cmp_type_list = typename wecs::type_list<CShape, CTransform, CCylinder>;
//public:
//
//	static uint32_t create(CShape eShape,
//						   CTransform eTransform,
//						   CCylinder eCylinder);
//
//	static float getArea(const CCylinder& eCylinder);
//
//	static bool intersect(const CTransform& eWorld,
//						  const CCylinder& eCylinder,
//						  const DRayf& ray);
//
//	static bool intersect(const CTransform& eWorld,
//						  const CCylinder& eCylinder,
//						  HCompR<CShape> hShape,
//						  const DRayf& rayW, float& tHit,
//						  CInteractionSurface& surfInter);
//
//	static DBounds3f getBoundLocal(const CCylinder& eCylinder);
//
//	static DBounds3f getBoundWorld(const CCylinder& eCylinder,
//								   const CTransform& eWorld);
//
//public:
//	
//	float getArea(const HEntity e) override
//	{
//		return getArea(e.getComponent<CCylinder>());
//	}
//
//	bool intersect(const HEntity e,
//				   const DRayf& ray) override
//	{
//		return intersect(e.getComponent<CTransform>(),
//						 e.getComponent<CCylinder>(),
//						 ray);
//	}
//
//	bool intersect(const HEntity e, 
//				   const DRayf& ray, float& tHit, CInteractionSurface& surfInter) override
//	{
//		return intersect(e.getComponent<CTransform>(),
//						 e.getComponent<CCylinder>(),
//						 e.getComponentHandleR<CShape>(),
//						 ray, tHit, surfInter);
//	}
//
//	DBounds3f getBoundLocal(const HEntity e) override
//	{
//		return getBoundLocal(e.getComponent<CCylinder>());
//	}
//
//	DBounds3f getBoundWorld(const HEntity e) override
//	{
//		return getBoundWorld(e.getComponent<CCylinder>(),
//							 e.getComponent<CTransform>());
//	}
//	
//};
//
//
//WPBR_END
//
//
