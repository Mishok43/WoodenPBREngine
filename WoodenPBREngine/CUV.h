#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DVector.h"

WPBR_BEGIN

struct CUV
{
	DVector2f uv;

	DECL_MANAGED_DENSE_COMP_DATA(CUV, 16)
}; DECL_OUT_COMP_DATA(CUV)

WPBR_END



