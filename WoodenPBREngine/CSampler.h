#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DPoint.h"

WPBR_BEGIN
struct CSamples1D
{
	std::vector<float, AllocatorAligned<float>> data;
	uint32_t i;
	DECL_MANAGED_DENSE_COMP_DATA(CSamples1D, 1)
}; DECL_OUT_COMP_DATA(CSamples1D)

struct CSamples2D
	{
		std::vector<DPoint2f, AllocatorAligned<DPoint2f>> data;
		uint32_t i;
		DECL_MANAGED_DENSE_COMP_DATA(CSamples2D, 1)
	}; DECL_OUT_COMP_DATA(CSamples2D)

WPBR_END

