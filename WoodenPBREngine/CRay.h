#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DRay.h"

WPBR_BEGIN

class CRay: public DRay<float>
{
public:
	using base = typename DRay<float>;
	using base::operator();



	CRay(DRayf&& r):
		DRayf(std::move(r)){ }

	DECL_MANAGED_DENSE_COMP_DATA(CRay, 32);
}; 

WPBR_END

