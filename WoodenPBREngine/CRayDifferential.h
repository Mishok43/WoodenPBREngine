#pragma once
#include "pch.h"
#include "DRayDifferential.h"

WPBR_BEGIN

struct CRayDifferential : public DRayDifferentialf
{
	CRayDifferential() = default;
	CRayDifferential(DRayf ray):
		DRayDifferentialf(std::move(ray))
	{ }

	CRayDifferential(DRayDifferentialf ray):
		DRayDifferentialf(std::move(ray))
	{}

	DECL_MANAGED_DENSE_COMP_DATA(CRayDifferential, 16)
}; DECL_OUT_COMP_DATA(CRayDifferential)

WPBR_END