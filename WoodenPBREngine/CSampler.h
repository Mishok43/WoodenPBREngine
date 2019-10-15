#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DPoint.h"

WPBR_BEGIN
struct CSamples1D
{
	std::vector<float, AllocatorAligned2<float, 32>> data;
	uint32_t i=0;
	DECL_MANAGED_DENSE_COMP_DATA(CSamples1D, 1)
}; 

struct CSamples2D
	{
		std::vector<std::array<float, 2>, AllocatorAligned2<std::array<float, 2>, 32>> data;
		uint32_t i=0;

		DPoint2f next()
		{
			assert(i < data.size());
			DPoint2f res = DPoint2f(data[i][0], data[i][1]);
			i++;
			return res;
		}

		DECL_MANAGED_DENSE_COMP_DATA(CSamples2D, 1)
	}; 

WPBR_END

