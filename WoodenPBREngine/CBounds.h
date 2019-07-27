#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DBounds.h"

WPBR_BEGIN
struct CBounds3f: public DBounds<float, 3>
{
	DECL_MANAGED_DENSE_COMP_DATA(CBounds3f, 1024);
}; DECL_OUT_COMP_DATA(CBounds3f)

WPBR_END

