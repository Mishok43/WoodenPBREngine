#pragma once

#include "pch.h"
#include "CRay.h"
#include "WoodenMathLibrarry/DNormal.h"
#include "WoodenMathLibrarry/DVector.h"
#include "WoodenMathLibrarry/HSolver.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "CRayDifferential.h"

WPBR_BEGIN

struct CMedium;


struct CFullInteractionRequest: public CompDummy
{
	DECL_MANAGED_DENSE_COMP_DATA(CFullInteractionRequest, 16)
}; DECL_OUT_COMP_DATA(CFullInteractionRequest)

struct CInteractionRequest
{
	HEntity rayCastEntity;
	float tHitResult;
};

struct CRayCast
{
	DRayf ray;
	bool bSurfInteraction=true; 
	HEntity hCollision;
	std::vector<HEntity, AllocatorAligned<HEntity>> interactionEntities;
}; 

struct CInteraction
{
	DPoint3f p;
	DVector3f wo;
	HEntity hCollision;
	float time;

	DECL_MANAGED_DENSE_COMP_DATA(CInteraction, 16);
}; DECL_OUT_COMP_DATA(CInteraction)

struct CSurfaceInteraction: public CInteraction
{
	DNormal3f n;
	CSurfaceInteraction() = default;

	CSurfaceInteraction(
		DPoint3f p, 
		DVector2f uv, DVector3f wo,
		DVector3f dpdu, DVector3f dpdv,
		DVector3f dndu, DVector3f dndv,
		float time, HEntity hCollision):
		CInteraction{
			std::move(p), std::move(wo),
			hCollision, time
		},
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

	mutable DVector3f dpdx, dpdy;
	mutable float dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;

	void computeDifferentials(const DRayDifferentialf& ray)
	{
		float d = dot(n, p);
		float tx = -(dot(n, ray.difXRay.origin) - d) / dot(n, ray.difXRay.dir);
		DPoint3f px = ray.difXRay.origin + ray.difXRay.dir*tx;

		float ty = -(dot(n, ray.difYRay.origin) - d) / dot(n, ray.difYRay.dir);
		DPoint3f py = ray.difYRay.origin + ray.difYRay.dir*ty;

		dpdx = px - p;
		dpdy = py - p;

		int dim[2];
		if (std::abs(n.x()) > std::abs(n.y()) && std::abs(n.x()) > std::abs(n.z()))
		{
			dim[0] = 1; dim[1] = 2;
		}
		else if (std::abs(n.y()) > std::abs(n.z()))
		{
			dim[0] = 0; dim[1] = 2;
		}
		else
		{
			dim[0] = 0; dim[1] = 1;
		}

		float A[2][2] = {
			{dpdu[dim[0]], dpdv[dim[0]]},
			{dpdu[dim[1]], dpdv[dim[1]]}
		};

		float Bx[2] = {
			px[dim[0]] - p[dim[0]],
			px[dim[1]] - p[dim[1]]
		};

		float By[2] = {
			py[dim[0]] - p[dim[0]],
			py[dim[1]] - p[dim[1]]
		};

		if (!Solvers::solveLinearSystem2x2(A, Bx, &dudx, &dvdx))
		{
			dudx = dvdx = 0;
		}

		if (!Solvers::solveLinearSystem2x2(A, Bx, &dudy, &dvdy))
		{
			dudy = dvdy = 0;
		}
	}


	//void computeDifferentialsForSpecularReflection(const DRayDifferentialf& rd)
	//{
	//	rd.difXRay.origin = rd.origin + rd.dpdx;
	//	rd.difYRay.origin = rd.origin + rd.dpdy;

	//	DNormal3f dndx = rd.shading.dndu * rd.dudx +
	//		rd.shading.dndv * rd.dvdx;
	//	DNormal3f dndy = rd.shading.dndu * rd.dudy +
	//		rd.shading.dndv * rd.dvdy;

	//	DVector3f dwodx = -rd.difXRay.dir - wo, dwody = -rd.difYRay.dir - wo;
	//	float dDNdx = dot(dwodx, shading.n) + dot(wo, dndx);
	//	float dDNdy = dot(dwody, shading.n) + dot(wo, dndy);
	//	rd.difXRay.dir = wi - dwodx +
	//		DVector3f(dndx*dot(wo, shading.n) + shading.n*dDNdx)*2;
	//	rd.difYRay.dir = wi - dwody +
	//		DVector3f(dndy*dot(wo, shading.n) + shading.n*dDNdy)*2;

	//}

	void setShadingGeometry(DVector3f dpdu, DVector3f dpdv,
							DVector3f dndu, DVector3f dndv,
							bool shadingOrientatinIsAuthoritive)
	{
		shading.n = DNormal3f(normalize(cross(dpdu, dpdv)));
		shading.dpdu = std::move(dpdu);
		shading.dpdv = std::move(dpdv);
		shading.dndu = std::move(dndu);
		shading.dndv = std::move(dndv);

		//const CShape& surfShape = hSurfShape.get();
		//if (surfShape.bReverseOrientation ^ surfShape.bTransformSwapsHandedness)
		//{
		//	shading.n *= -1;
		//}

		if (shadingOrientatinIsAuthoritive)
		{
			n = faceForward(n, shading.n);
		}
		else
		{
			shading.n = faceForward(shading.n, n);
		}
	}

	DECL_MANAGED_DENSE_COMP_DATA(CSurfaceInteraction, 8);
}; DECL_OUT_COMP_DATA(CSurfaceInteraction)




class JobComputeDifferentialsForSurfInter : public JobParallazible
{
	constexpr static uint32_t slice = 128;

	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return min(nWorkThreads, (queryComponentsGroup<CRayDifferential, CSurfaceInteraction>().size() + slice - 1) / slice);
	}

	void update(WECS* ecs, uint8_t iThread) override
	{

		uint32_t nRequests = queryComponentsGroup<CRayDifferential, CSurfaceInteraction>().size();
		uint32_t sliceSize = (nRequests + getNumThreads() - 1) / getNumThreads();

		ComponentsGroupSlice<CRayDifferential, CSurfaceInteraction> differentials =
			queryComponentsGroupSlice<CRayDifferential, CSurfaceInteraction>(Slice(iThread * sliceSize, sliceSize));

		for_each([&](HEntity hEntity,
				 const CRayDifferential& ray,
				 CSurfaceInteraction& si)
		{
			si.computeDifferentials(ray);
		}, differentials);
	}
};



WPBR_END

