#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DRay.h"

WPBR_BEGIN

class CRay: public DRay<float>
{
public:
	using base = typename DRay<float>;
	using base::operator();

	CRay(DVector<float, 3> o, DVector<float, 3> d):
		DRay(std::move(o), std::move(d))
	{ }


	DECL_MANAGED_DENSE_COMP_DATA(CRay, 32);
}; DECL_OUT_COMP_DATA(CRay)

WPBR_END

