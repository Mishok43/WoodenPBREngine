#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DBounds.h"

WPBR_BEGIN
struct CBounds : public DBounds3f
{
	CBounds() = default;

	CBounds(DBounds3f bounds):
		DBounds3f(std::move(bounds))
	{}

	DECL_MANAGED_FLAT_COMP_DATA(CBounds, 16);
}; DECL_OUT_COMP_DATA(CBounds)

WPBR_END

