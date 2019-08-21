#pragma once
#include "pch.h"

WPBR_BEGIN

struct CMedium
{


	DECL_UNMANAGED_DENSE_COMP_DATA(CMedium, 16);
}; DECL_OUT_COMP_DATA(CMedium)


class SMedium
{
public:
	virtual CSpectrum Tr(const DRay& ray, CSampler& sampler) const = 0;
};

WPBR_END