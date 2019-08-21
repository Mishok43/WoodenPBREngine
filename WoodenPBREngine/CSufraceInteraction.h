#pragma once

#include "pch.h"
#include "CRay.h"
#include "WoodenMathLibrarry/DNormal.h"
#include "WoodenMathLibrarry/DVector.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "CShape.h"

WPBR_BEGIN

struct CMedium;
struct CShape;

struct CInteraction
{
	DPoint3f p;
	DVector3f pError;
	DVector3f wo;
	DNormal3f n;
	CMedium* medium;
	float time;

	bool isSurfaceInteraction() const { return n != DNormal3f(); }

	DECL_MANAGED_DENSE_COMP_DATA(CInteraction, 8);
}; DECL_OUT_COMP_DATA(CInteraction)

struct CInteractionSurface: public CInteraction
{
	CInteractionSurface(){ }

	CInteractionSurface(
		DPoint3f p, DVector3f pError,
		DVector2f uv, DVector3f wo,
		DVector3f dpdu, DVector3f dpdv,
		DVector3f dndu, DVector3f dndv,
		float time, HCompR<CShape> hSurfShape):
		CInteraction{
			std::move(p), 
			std::move(pError), std::move(wo),
			DNormal3f(normalize(cross(dpdu, dpdv))), nullptr, time
		},
		uv(std::move(uv)),
		dpdu(std::move(dpdu)), dpdv(std::move(dpdv)),
		dndu(std::move(dndu)), dndv(std::move(dndv)),
		hSurfShape(hSurfShape)
	{}

	DVector2f uv;
	DVector3f dpdu, dpdv;
	DNormal3f dndu, dndv;
	
	struct
	{
		DNormal3f n;
		DVector3f dpdu, dpdv;
		DVector3f dndu, dndv;
	} shading;

	HCompR<CShape> hSurfShape;

	void setShadingGeometry(DVector3f dpdu, DVector3f dpdv,
							DVector3f dndu, DVector3f dndv,
							bool shadingOrientatinIsAuthoritive)
	{
		shading.n = DNormal3f(normalize(cross(dpdu, dpdv)));
		shading.dpdu = std::move(dpdu);
		shading.dpdv = std::move(dpdv);
		shading.dndu = std::move(dndu);
		shading.dndv = std::move(dndv);

		const CShape& surfShape = hSurfShape.get();
		if (surfShape.bReverseOrientation ^ surfShape.bTransformSwapsHandedness)
		{
			shading.n *= -1;
		}

		if (shadingOrientatinIsAuthoritive)
		{
			n = faceForward(n, shading.n);
		}
		else
		{
			shading.n = faceForward(shading.n, n);
		}
	}

	DECL_MANAGED_DENSE_COMP_DATA(CInteractionSurface, 8);
}; DECL_OUT_COMP_DATA(CInteractionSurface)



WPBR_END

