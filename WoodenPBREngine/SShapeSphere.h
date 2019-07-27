//#pragma once
//
//#include "pch.h"
//#include "CShape.h"
//#include "CBounds.h"
//#include "CRay.h"
//#include "CSufraceInteraction.h"
//#include "MEngine.h"
//
//WPBR_BEGIN
//
//
//class SShapeSphere
//{
//	using cmp_type_list = typename wecs::type_list<CShape, CTransform>;
//
//	struct Entity
//	{
//		CShape& shape;
//		CTransform& transform;
//		CBounds3f& bounds;
//	};
//
//public:
//	static uint32_t createSphere(CShape shape,
//								 CSphere sphere,
//								 CTransform transform)
//	{
//		MEngine& engine = MEngine::getInstance();
//		uint32_t hEntity = engine.createEntity();
//		engine.addComponent<CTransform>(hEntity, std::move(transform));
//		engine.addComponent<CShape>(hEntity, std::move(shape));
//		engine.addComponent<CSphere>(hEntity, std::move(sphere));
//	
//		return hEntity;
//	}
//
//	float getArea()
//	{
//
//	}
//
//	static bool Intersect(const Entity& e,
//						  const CRay& ray) 
//	{
//		float tHit = ray.tMax;
//		CInteractionSurface surfInter;
//		return Intersect(e, ray, tHit, surfInter);
//	}
//
//	static bool Intersect(const Entity& e, 
//						  const CRay& ray, float& tHit,
//						  CInteractionSurface& surfInter)
//	{
//		
//	}
//
//	static CBounds3f getWorldBound(const Entity& e)
//	{
//		return e.transform(e.bounds);
//	}
//};
//
//WPBR_END
//
