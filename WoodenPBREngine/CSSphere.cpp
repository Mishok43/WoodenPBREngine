#include "pch.h"
#include "CSSphere.h"
#include "MEngine.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/Samplers.h"
#include <algorithm>
#include "WoodenMathLibrarry/Utils.h"

WPBR_BEGIN

using namespace SSphere;

DECL_OUT_COMP_DATA(CInteractionSphere)
DECL_OUT_COMP_DATA(CSphere)

void JobProcessSphereSurfInteractionRequests::update(WECS* ecs, uint8_t iThread)
{
	uint32_t nCollisions = queryComponentsGroup<CSphere>().size();
	uint32_t sliceSize = (nCollisions + getNumThreads()-1) /getNumThreads();
	uint32_t iStart = iThread * sliceSize;

	ComponentsGroup<CSphere, CTransform, CInteractionRequests> requests = queryComponentsGroup<CSphere,  CTransform, CInteractionRequests>();

	for_each([ecs](HEntity hEntity,
			 CSphere& sphere,
			 CTransform& world,
			 CInteractionRequests& interactionRequests)
	{
		for (uint32_t i = 0; i < interactionRequests.size(); i++)
		{
			HEntity hRequest = interactionRequests[i];
			CInteractionRequest& request = ecs->getComponent<CInteractionRequest>(hRequest);
			const DRayf& rayW = request.ray;
			float eRadius = sphere.radius;


			CInteractionSphere interactionSphere;
			float tHit;
			if (intersect(rayW, sphere, world, interactionSphere, tHit))
			{
				request.tHitResult = tHit;
				ecs->addComponent<CInteractionSphere>(hRequest, std::move(interactionSphere));
			}
			else
			{
				request.tHitResult = 0.0f;
			}
		}
		interactionRequests.clear();
	}, requests);
}

void JobProcessSphereFullInteractionRequests::update(WECS* ecs, uint8_t iThread)
{
	ComponentsGroup<CSphere,  CTransform, CInteractionFullRequests> requests =
		queryComponentsGroup<CSphere, CTransform, CInteractionFullRequests>();

	for_each([ecs](HEntity hEntity,
			 CSphere& sphere,
			 CTransform& world,
			 CInteractionFullRequests& interactionRequests)
	{
		for (uint32_t i = 0; i < interactionRequests.size(); i++)
		{
			HEntity hRequest = interactionRequests[i];

			CInteractionRequest& request = ecs->getComponent<CInteractionRequest>(hRequest);
			CInteractionSphere& requestSphere = ecs->getComponent<CInteractionSphere>(hRequest);
			const DRayf& rayW = request.ray;

			CSurfaceInteraction si = computeSurfInteraction(request.hShape, sphere, world, requestSphere);
			ecs->addComponent<CSurfaceInteraction>(request.hRayCast, std::move(si));
		}

		interactionRequests.clear();
	}, requests);
}

void JobProcessSphereFullInteractionRequests::finish(WECS* ecs)
{
	ecs->clearComponents<CInteractionSphere>();
}

bool SSphere::intersect(
	const DRayf& rayW,
	const CSphere& sphere,
	const CTransform& world,
	CInteractionSphere& interactionSphere,
	float& tHit)
{
	const float& radius = sphere.radius;
 	DRayf rayL = world(rayW, INV_TRANFORM);
	float a = rayL.dir.length2();
	float b = 2 * (dot(rayL.dir, rayL.origin));
	float c = rayL.origin.length2() - radius * radius;

	float t0, t1;
	if (!getRootsQuadraticEquation(a, b, c, t0, t1))
	{
		return false;
	}

	if (t1 <= 0)
	{
		return false;
	}


	tHit = t0;
	if (tHit <= 0)
	{
		tHit = t1;
	}

	DPoint3f pHitL = rayL(tHit);
	rayL.t = tHit;
	interactionSphere.pHitL = pHitL;
	interactionSphere.rayL = rayL;
	return true;
}

CSurfaceInteraction SSphere::computeSurfInteraction(
	HEntity hSphere,
	const CSphere& sphere, 
	const CTransform& world, 
	const CInteractionSphere& interactionSphere)
{

	const DVector3f& pHit = interactionSphere.pHitL;
	const DRayf& rayL = interactionSphere.rayL;
	const float& radius = sphere.radius;

	/*if (pHit.x() == 0 && pHit.y() == 0)
	{
		pHit.x() = 1e-5f*eRadius;
	}*/

	float phi = std::atan2(pHit.y(), pHit.x());
	if (phi < 0.0)
	{
		phi += 2 * PI;
	}

	float u = phi /( 2 * PI);
	float theta = std::acos(pHit.z() / radius);
	float v = theta / PI;

	float invZRadius = 1.0 / std::sqrt(pHit.x()*pHit.x() + pHit.y()*pHit.y());
	float phiCos = pHit.x()*invZRadius;
	float phiSin = pHit.y()*invZRadius;

	DVector3f dpdu = DVector3f(-pHit.y(), pHit.x(), 0.0);
	dpdu *= 2 * PI;

	DVector3f dpdv = DVector3f(pHit.z()*phiCos, pHit.z()*phiSin, -radius * sin(theta));
	dpdv *= PI;


	DVector3f d2Pduu = DVector3f(pHit.x(), pHit.y(), 0)*(-2 * PI * 2 * PI);
	DVector3f d2Pduv = DVector3f(-phiSin, phiCos, 0.0f)*PI* pHit.z() * 2 * PI;
	DVector3f d2Pdvv = DVector3f(pHit.x(), pHit.y(), pHit.z())*-PI * PI;

	float E = dot(dpdu, dpdu);
	float F = dot(dpdu, dpdv);
	float G = dot(dpdv, dpdv);
	DVector3f N = normalize(cross(dpdu, dpdv));
	float e = dot(N, d2Pduu);
	float f = dot(N, d2Pduv);
	float g = dot(N, d2Pdvv);

	float invEGF2 = 1 / (E * G - F * F);
	DNormal3f dndu = DNormal3f(dpdu*((f * F - e * G) * invEGF2) +
							   dpdv * ((e * F - f * E) * invEGF2));
	DNormal3f dndv = DNormal3f(dpdu*((g * F - f * G) * invEGF2) +
							   dpdv * ((f * F - g * E) * invEGF2));

	auto res = CSurfaceInteraction(pHit, DVector2f(u, v), -rayL.dir,
									 dpdu, dpdv, dndu, dndv, rayL.t, hSphere);

	//res.n = res.shading.n = DNormal3f(-rayL.dir);
	res = world(res);

	assert(res.n.length2() >= 0.98f && res.n.length2() <= 1.02f);

	return res;
}

CTextureMappedPoint SSphere::mapUV(
	const CSphere& sphere,
	const CTransform& world,
	const CSurfaceInteraction& si)
{
	CTransform world2 = world;
	world2.mInv().setScale(world2.mInv().getScale() / sphere.radius);
	return TextureMappingSphere::map(si, world2);
}


uint32_t SSphere::create(CTransform transform,
					   CSphere sphere)
{
	MEngine& engine = MEngine::getInstance();
	uint32_t hEntity = engine.createEntity();
	engine.addComponent<CTransform>(hEntity, std::move(transform));
	engine.addComponent<CSphere>(hEntity, std::move(sphere));
	engine.addComponent<CBounds>(hEntity);
	engine.addComponent<CCentroid>(hEntity);
	engine.addComponent<CMapUVRequests>(hEntity);
	engine.addComponent<CInteractionRequests>(hEntity);
	engine.addComponent<CInteractionFullRequests>(hEntity);

	return hEntity;
}

float SSphere::getArea(const CSphere& eSphere)
{
	return 4*PI*eSphere.radius*eSphere.radius;
}


float SSphere::pdf(
	const CSphere& sphere, 
	const CTransform& world, 
	const CInteraction& inter, 
	const DVector3f& wi)
{
	DPoint3f pCenteroidW = world(DPoint3f(0.0, 0.0, 0.0));
	float sinThetaMax2 = sphere.radius * sphere.radius / (inter.p, pCenteroidW).length2();
	float cosThetaMax = std::sqrt(max(0.0f, 1 - sinThetaMax2));
	return coneUniformPDF(cosThetaMax);
	/*DRayf ray;
	ray.origin = inter.p;
	ray.dir = wi;

	CInteractionSphere interSphere;
	float tHit;
	if (!intersect(ray, sphere, world, interSphere, tHit))
	{
		return 0;
	}

	CSurfaceInteraction si = computeSurfInteraction(sphere, world, interSphere);
	float area = getArea(sphere);
	float p = (si.p - inter.p).length2()/(absDot(si.n, -wi)*area);
	return p;*/
}

DPoint3f SSphere::sample(
	const CSphere& sphere,
	const CTransform& world,
	const CInteraction& interac,
	const DPoint2f& u,
	float& p)
{
	DPoint3f pCenteroidW = world(DPoint3f(0.0, 0.0, 0.0));
	DVector3f wc = normalize(pCenteroidW - interac.p);

	DVector3f wcX, wcY;
	makeBasisByVector(wc, wcX, wcY);

	const float& radius = sphere.radius;
	float sinThetaMax2 = sphere.radius*sphere.radius / (interac.p - pCenteroidW).length2();
	float cosThetaMax = std::sqrt(max(0.0f, 1.0 - sinThetaMax2));
	float cosTheta = u[0]*(1.0 - cosThetaMax)+cosThetaMax;
	float sinTheta = std::sqrt(max(0.0f, 1.0 - cosTheta * cosTheta));
	float phi = u[1] * 2 * PI;

	float dc = (interac.p- pCenteroidW).length();
	float ds = dc * cosTheta -
		std::sqrt(max(0.0f,
				  radius * radius - dc * dc * sinTheta * sinTheta));
	float cosAlpha = (dc * dc + radius * radius - ds * ds) /
		(2 * dc * radius);
	float sinAlpha = std::sqrt(max(0.0f, 1 - cosAlpha * cosAlpha));
	
	DVector3f nL = sphericalToCasterian(sinAlpha, cosAlpha, phi, -wcX, -wcY, -wc);
	DPoint3f pL = nL * sphere.radius;

	p = coneUniformPDF(cosThetaMax);
	return world(pL);
}

DBounds3f SSphere::getBoundLocal(const CSphere& eSphere)
{
	return DBounds3f(DPoint3f(-eSphere.radius, -eSphere.radius, -eSphere.radius),
					 DPoint3f(eSphere.radius, eSphere.radius, eSphere.radius));
}

DBounds3f SSphere::getBoundWorld(const CSphere& eSphere,
							   const CTransform& eWorld)
{
	return eWorld(getBoundLocal(eSphere));
}


WPBR_END

