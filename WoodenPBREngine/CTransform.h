#pragma once

#include "pch.h"
#include "CSufraceInteraction.h"
#include "WoodenMathLibrarry/DTransform.h"

WPBR_BEGIN

//template<typename T>
class CTransform: public DTransform<float>
{
	using Vector = typename DVector<float, 3>;
	using Quaternion = typename DQuaternion<float>;
	using Matrix = typename DMatrix<float>;
public:
	using base = typename DTransform<float>;

	using base::operator ();

	CTransform(const Vector& trans, const Vector& scale, const Quaternion& rotation):
		DTransform(trans, scale, rotation)
	{ }

	CTransform(Matrix m, Matrix mInv) :
		DTransform(std::move(m), std::move(mInv))
	{}

	inline CInteractionSurface operator()(const CInteractionSurface& interSurf) const
	{
		CInteractionSurface res;
		res.dndu = (*this)(interSurf.dndu);
		res.dndv = (*this)(interSurf.dndv);
		res.dpdu = (*this)(interSurf.dpdu); 
		res.dpdv = (*this)(interSurf.dpdv);
		
		//res.uv = (*this)(interSurf.uv);
		res.n = (*this)(interSurf.n);
		res.wo = (*this)(interSurf.wo);

		res.shading.dndu = (*this)(interSurf.shading.dndu);
		res.shading.dndv = (*this)(interSurf.shading.dndv);
		res.shading.dpdu = (*this)(interSurf.shading.dpdu);
		res.shading.dpdv = (*this)(interSurf.shading.dpdv);
		res.shading.n = (*this)(interSurf.shading.n);

		return res;
	}

	DECL_MANAGED_DENSE_COMP_DATA(CTransform, 1024)
}; DECL_OUT_COMP_DATA(CTransform)

//using CTransformf = typename CTransform<float>;
//using CTransformd = typename CTransform<double>;
//
//DECL_OUT_COMP_DATA(CTransformf)
//DECL_OUT_COMP_DATA(CTransformd)

WPBR_END