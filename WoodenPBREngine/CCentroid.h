#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/DBounds.h"

WPBR_BEGIN
struct CCentroid : public DPoint3f
{
	CCentroid(const DBounds3f& bounds) :
		DPoint3f(bounds.pMax*0.5+bounds.pMin*0.5)
	{}

	DECL_UNMANAGED_FLAT_SHARED_COMP_DATA(CCentroid, 16);
}; DECL_OUT_COMP_DATA(CCentroid)

WPBR_END

