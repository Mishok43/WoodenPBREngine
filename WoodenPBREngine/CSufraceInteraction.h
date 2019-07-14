#pragma once

#include "pch.h"
#include "CRay.h"
#include "WoodenMathLibrarry/DNormal.h"
#include "WoodenMathLibrarry/DVector.h"
#include "WoodenMathLibrarry/DPoint.h"

WPBR_BEGIN


class CMedium;
class CPrimitive;

struct CInteraction
{
	DPoint3f p;
	DVector3f pError;
	DVector3f wo;
	DNormal3f n;
	wecs::CompPointer<CMedium> medium;
	float time;

	bool isSurfaceInteraction() const { return n != DNormal3f(); }
};

struct CInteractionSurface: public CInteraction
{
	CInteractionSurface(
		DPoint3f p, DVector3f pError,
		DVector2f uv, DVector3f wo,
		DVector3f dpdu, DVector3f dpdv,
		DVector3f dndu, DVector3f dndv,
		float time, wecs::CompPointer<CPrimitive>
	): CInteraction{std::move(p), std::move(pError), std::move(wo),
		DNormal3f(normalize(cross(dpdu, dpdv))), nullptr, time},
		uv(std::move(uv)),
		dpdu(std::move(dpdu)), dpdv(std::move(dpdv)),
		dndu(std::move(dndu)), dndv(std::move(dndv))
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

	wecs::CompPointer<CPrimitive> primitive;

	void setShadingGeometry(DVector3f dpdu, DVector3f dpdv,
							DVector3f dndu, DVector3f dndv,
							bool shadingOrientatinIsAuthoritive)
	{
		shading.n = DNormal3f(normalize(cross(dpdu, dpdv)));
		shading.dpdu = std::move(dpdu);
		shading.dpdv = std::move(dpdv);
		shading.dndu = std::move(dndu);
		shading.dndv = std::move(dndv);

		if (primitive && (primitive.ptr->bReverseOrientation | primitive.ptr->transformSwapsHandedness))
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


};



WPBR_END

