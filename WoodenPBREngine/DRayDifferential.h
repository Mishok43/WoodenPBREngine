#pragma once

#include "pch.h"
#include "WoodenMathLibrarry/DRay.h"

WPBR_BEGIN
template<typename T, uint8_t alignment = sse_alignment_size_v<__m_t<T>>>
class alignas(alignment) DRayDifferential: public DRay<T>
{
	using vector_type = typename DVector<T, 4>;

	DRayDifferential(DRay ray) :
		DRay(std::move(ray));
	{}

	void scaleDifferentials(T s)
	{
		difXRay.origin = vector_type::mAdd(difXRay.origin - origin, s, origin);
		difXRay.dir = vector_type::mAdd(difXRay.dir - dir, s, dir);

		difYRay.origin = vector_type::mAdd(difYRay.origin - origin, s, origin);
		difYRay.dir = vector_type::mAdd(difYRay.dir - dir, s, dir);
	}

protected:
	DRay<T> difXRay;
	DRay<T> difYRay;
};

WPBR_END

