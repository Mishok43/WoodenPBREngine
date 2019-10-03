#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DPoint.h"

WPBR_BEGIN
struct CSamples1D
{
	std::vector<float> data;
	uint32_t i=0;
	DECL_MANAGED_DENSE_COMP_DATA(CSamples1D, 1)
}; 

struct CSamples2D
	{
		std::vector<DPoint2f, AllocatorAligned2<DPoint2f>> data;
		uint32_t i=0;
		DECL_MANAGED_DENSE_COMP_DATA(CSamples2D, 1)
	}; 

WPBR_END

