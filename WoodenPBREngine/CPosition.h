#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DPoint.h"


WPBR_BEGIN

struct CPosition: public DPoint3f
{
	DECL_UNMANAGED_DENSE_COMP_DATA(CPosition, 16)
}; DECL_OUT_COMP_DATA(CPosition)

WPBR_END

