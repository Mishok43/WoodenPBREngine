#pragma once

#include "pch.h"
#include "CSufraceInteraction.h"
#include "WoodenMathLibrarry/DTransform.h"

WPBR_BEGIN

template<typename T>
class MTransform: public DTransform<T>
{
public:


	
	inline CInteractionSurface operator()(const CInteractionSurface& interSurf) const
	{
		CInteractionSurface res;
		(*this)(res.dndu); (*this)(res.dndv);
		(*this)(res.dpdu); (*this)(res.dpdv);
		(*this)(res.uv);
		(*this)(res.n);
		(*this)(res.wo);

		(*this)(res.shading.dndu); (*this)(res.shading.dndv);
		(*this)(res.shading.dpdu); (*this)(res.shading.dpdv);
		(*this)(res.shading.n);

	}
};

WPBR_END