#pragma once

#include "pch.h"
#include "CMaterial.h"
#include "MEngine.h"

WPBR_BEGIN

struct CPrimitive
{


	DECL_MANAGED_DENSE_COMP_DATA(CPrimitive, 16)
}; DECL_OUT_COMP_DATA(CPrimitive)

struct CPrimitiveGeometric
{

	DECL_MANAGED_DENSE_COMP_DATA(CPrimitiveGeometric, 16)
}; DECL_OUT_COMP_DATA(CPrimitiveGeometric)

class SPrimitiveGeometric
{
	// CPrimitiveGeometric, HComp<CMaterial>, HComp<CAreaLight>, HComp<CShape>

	template<typename CShapeT>
	static bool intersect(const HComp<CShapeT> eHShape, 
						  const CRay& ray, 
						  CInteractionSurface& surfInter)
	{
		MEngine& mengine = MEngine::getInstance();
		CShapeT& eShape = mengine.getComponent<CShapeT>(eHShape);
		float tHit;
		if (!CShapeT::intersect(ray, &tHit, surfInter))
		{
			return false;
		}

		r.tMax = tHit;
		surfInter.
	}
};

WPBR_END

