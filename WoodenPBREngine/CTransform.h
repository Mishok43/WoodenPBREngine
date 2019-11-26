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

	CTransform(const DTransform& t) :
		DTransform(t)
	{
	}

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
		res.hCollision = surfInter.hCollision;
		res.time = surfInter.time;
		res.dndu = (*this)(surfInter.dndu);
		res.dndv = (*this)(surfInter.dndv);
		res.dpdu = (*this)(surfInter.dpdu); 
		res.dpdv = (*this)(surfInter.dpdv);
		res.p = (*this)(surfInter.p);
		res.dpdx = (*this)(surfInter.dpdx);
		res.dpdy = (*this)(surfInter.dpdx);
		
		//res.uv = (*this)(surfInter.uv);
		res.n = normalize((*this)(surfInter.n));

		res.wo = normalize((*this)(surfInter.wo));

		res.shading.dndu = (*this)(surfInter.shading.dndu);
		res.shading.dndv = (*this)(surfInter.shading.dndv);
		res.shading.dpdu = (*this)(surfInter.shading.dpdu);
		res.shading.dpdv = (*this)(surfInter.shading.dpdv);
		res.shading.n = normalize((*this)(surfInter.shading.n));

		return res;
	}

	inline DRayDifferentialf operator()(const DRayDifferentialf& rayDif) const
	{
		DRayDifferentialf res;
		res.origin = (*this)(rayDif.origin);
		res.dir = normalize((*this)(rayDif.dir));
		res.difXRay = ((*this)(rayDif.difXRay));
		res.difYRay = ((*this)(rayDif.difYRay));
		return res;
	}

	DECL_MANAGED_DENSE_COMP_DATA(CTransform, 1024)
}; 


struct CTransformCameraScreen : public CTransform
{
	
	CTransformCameraScreen(CTransform&& c):
		CTransform(std::move(c)){ }

	DECL_MANAGED_DENSE_COMP_DATA(CTransformCameraScreen, 1)
};

struct CTransformRasterCamera : public CTransform
{
		CTransformRasterCamera(CTransform&& c) :
			CTransform(std::move(c))
		{
		}
	DECL_MANAGED_DENSE_COMP_DATA(CTransformRasterCamera, 1)
}; 

struct CTransformScreenRaster : public CTransform
{
		CTransformScreenRaster(CTransform&& c) :
			CTransform(std::move(c))
		{
		}
	DECL_MANAGED_DENSE_COMP_DATA(CTransformScreenRaster, 1)
}; 


//using CTransformf = typename CTransform<float>;
//using CTransformd = typename CTransform<double>;
//
//DECL_OUT_COMP_DATA(CTransformf)
//DECL_OUT_COMP_DATA(CTransformd)

WPBR_END