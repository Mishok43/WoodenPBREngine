#pragma once

#include "pch.h"
#include "CSufraceInteraction.h"
#include "WoodenMathLibrarry/DTransform.h"
#include "DRayDifferential.h"

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

	CTransform() = default;

	CTransform(DTransform&& t):
		DTransform(std::move(t))
	{ }

	CTransform(const Vector& trans, const Vector& scale, const Quaternion& rotation):
		DTransform(trans, scale, rotation)
	{ }

	CTransform(Matrix m, Matrix mInv) :
		DTransform(std::move(m), std::move(mInv))
	{}


	inline CSurfaceInteraction operator()(const CSurfaceInteraction& surfInter) const
	{
		CSurfaceInteraction res;
		res.dndu = (*this)(surfInter.dndu);
		res.dndv = (*this)(surfInter.dndv);
		res.dpdu = (*this)(surfInter.dpdu); 
		res.dpdv = (*this)(surfInter.dpdv);
		
		//res.uv = (*this)(surfInter.uv);
		res.n = (*this)(surfInter.n);

		res.wo = (*this)(surfInter.wo);

		res.shading.dndu = (*this)(surfInter.shading.dndu);
		res.shading.dndv = (*this)(surfInter.shading.dndv);
		res.shading.dpdu = (*this)(surfInter.shading.dpdu);
		res.shading.dpdv = (*this)(surfInter.shading.dpdv);
		res.shading.n = (*this)(surfInter.shading.n);

		return res;
	}

	inline DRayDifferentialf operator()(const DRayDifferentialf& rayDif) const
	{
		DRayDifferentialf res;
		res.origin = (*this)(rayDif.origin);
		res.dir = (*this)(rayDif.dir);
		res.difXRay = (*this)(rayDif.difXRay);
		res.difYRay = (*this)(rayDif.difYRay);
		return res;
	}

	DECL_MANAGED_DENSE_COMP_DATA(CTransform, 1024)
}; DECL_OUT_COMP_DATA(CTransform)


struct CTransformCameraScreen : public CTransform
{
	using base = typename CTransform;
	using base::operator ();
	DECL_MANAGED_DENSE_COMP_DATA(CTransformCameraScreen, 1)
}; DECL_OUT_COMP_DATA(CTransformCameraScreen)

struct CTransformRasterCamera : public CTransform
{
	using base = typename CTransform;
	using base::operator ();

	DECL_MANAGED_DENSE_COMP_DATA(CTransformRasterCamera, 1)
}; DECL_OUT_COMP_DATA(CTransformRasterCamera)

struct CTransformScreenRaster : public CTransform
{
	using base = typename CTransform;
	using base::operator ();
	DECL_MANAGED_DENSE_COMP_DATA(CTransformScreenRaster, 1)
}; DECL_OUT_COMP_DATA(CTransformScreenRaster)


//using CTransformf = typename CTransform<float>;
//using CTransformd = typename CTransform<double>;
//
//DECL_OUT_COMP_DATA(CTransformf)
//DECL_OUT_COMP_DATA(CTransformd)

WPBR_END