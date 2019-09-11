#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DVector.h"

WPBR_BEGIN

struct CTangent
{
	DVector3f tangent;

	DECL_UNMANAGED_DENSE_COMP_DATA(CTangent, 16)
}; DECL_OUT_COMP_DATA(CTangent)

WPBR_END

