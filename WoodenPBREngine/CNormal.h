#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DNormal.h"

WPBR_BEGIN

struct CNormal: public DNormal3f
{
	DECL_UNMANAGED_DENSE_COMP_DATA(CNormal, 16)
}; DECL_OUT_COMP_DATA(CNormal)

WPBR_END

