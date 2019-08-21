#pragma once

#include "pch.h"
#include "CTransform.h"
#include "WoodenMathLibrarry/DBounds.h"
#include "WoodenMathLibrarry/DRay.h"

WPBR_BEGIN

struct CTransform;

class SShape
{
public:
	virtual float getArea(const HEntity e) = 0;
	virtual bool intersect(const HEntity e,
						   const DRayf& ray) = 0;
	virtual bool intersect(const HEntity e, const DRayf& ray,
						   float& tHit, CInteractionSurface& surfInter) = 0;
	virtual DBounds3f getBoundLocal(const HEntity e) = 0;
	virtual DBounds3f getBoundWorld(const HEntity e) = 0;
};

WPBR_END

