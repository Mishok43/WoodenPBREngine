#include "CSSphere.h"
#include "MEngine.h"
#include "WoodenMathLibrarry/DPoint.h"

WPBR_BEGIN

void JobProcessSphereInteractionRequests::update(WECS* ecs, uint8_t iThread)
{
	uint32_t nCollisions = queryComponentsGroup<CInteractionSphere>().size<CInteractionSphere>();
	uint32_t sliceSize = (nCollisions + nThreads - 1) / nThreads;
	uint32_t iStart = iThread * sliceSize;

	ComponentsGroupSlice<CInteractionSphere, CInteractionRequest> requests =
		queryComponentsGroupSlice<CInteractionSphere, CInteractionRequest>(Slice(iStart, sliceSize));

	for_each([ecs](HEntity hEntity,
			 CInteractionSphere& interactionSphere,
			 CInteractionRequest& interactionRequest)
	{
		const CRayCast& rayCast = ecs->getComponent<CRayCast>(interactionRequest.rayCastEntity);
		const DRayf& rayW = rayCast.ray;

		const CSphere& triangle = ecs->getComponent<CSphere>(interactionSphere.hSphere);
		float eRadius = triangle.radius;
		const CTransform& eWorld = ecs->getComponent<CTransform>(interactionSphere.hSphere);
		
		float phi;
		DPoint3f pHit;

		DVector3f oErr, dErr;
		DRayf rayL = eWorld(rayW, INV_TRANFORM);

		float a = rayL.dir.length2();
		float b = 2 * (dot(rayL.dir, rayL.origin));
		float c = rayL.origin.length2() - eRadius * eRadius;

		float t0, t1;
		if (!Solvers::getRootsQuadraticEquation(a, b, c, t0, t1))
		{
			interactionRequest.tHitResult = 0.0;
			return;
		}

		if (t0 > rayL.tMax || t1 <= 0)
		{
			interactionRequest.tHitResult = 0.0;
			return;
		}
			

		float tHit = t0;
		if (tHit <= 0)
		{
			tHit = t1;
			if (tHit > rayL.tMax)
			{
				interactionRequest.tHitResult = 0.0;
				return;
			}
		}

		DPoint3f pHitL = rayL(tHit);
		interactionRequest.tHitResult = tHit;
		interactionSphere.pHitL = pHitL;
		interactionSphere.rayL = rayL;
	}, requests);
}

void JobProcessSphereInteractionRequests::update(WECS* ecs, uint8_t iThread)
{
	uint32_t nCollisions = queryComponentsGroup<CFullInteractionRequest>().size<CFullInteractionRequest>();
	uint32_t sliceSize = (nCollisions + nThreads - 1) / nThreads;
	uint32_t iStart = iThread * sliceSize;

	ComponentsGroupSlice<CFullInteractionRequest, CInteractionSphere, CInteractionRequest> requests =
		queryComponentsGroupSlice<CFullInteractionRequest, CInteractionSphere, CInteractionRequest>(Slice(iStart, sliceSize));

	for_each([ecs](HEntity hEntity,
			 CFullInteractionRequest&,
			 CInteractionSphere& interactionSphere,
			 const CInteractionRequest& interactionRequest)
	{

		const CSphere& sphere = ecs->getComponent<CSphere>(interactionSphere.hSphere);
		float eRadius = sphere.radius;
		const CTransform& eWorld = ecs->getComponent<CTransform>(interactionSphere.hSphere);

		DVector3f& pHit = interactionSphere.pHitL;
		DRayf& rayL = interactionSphere.rayL;

		if (pHit.x() == 0 && pHit.y() == 0)
		{
			pHit.x() = 1e-5f*eRadius;
		}

		float phi = std::atan2(pHit.y(), pHit.x());
		if (phi < 0.0)
		{
			phi += 2 * PI;
		}

		float u = phi / 2 * PI;
		float theta = std::acos(pHit.z() / eRadius);
		float v = theta / PI;

		float invZRadius = 1.0 / std::sqrt(pHit.x()*pHit.x() + pHit.y()*pHit.y());
		float phiCos = pHit.x()*invZRadius;
		float phiSin = pHit.y()*invZRadius;

		DVector3f dpdu = DVector3f(-pHit.y(), pHit.x(), 0.0);
		dpdu *= 2 * PI;

		DVector3f dpdv = DVector3f(pHit.z()*phiCos, pHit.z()*phiSin, -eRadius * sin(theta));
		dpdv *= PI;


		DVector3f d2Pduu = DVector3f(pHit.x, pHit.y, 0)*(-2 * PI * 2 * PI);
		DVector3f d2Pduv = DVector3f(-phiSin, phiCos, 0.)*PI* pHit.z * 2 * PI;
		DVector3f d2Pdvv = DVector3f(pHit.x, pHit.y, pHit.z)*-PI * PI;

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

		//const HComp<CShape> h = hShape;

		CSurfInteraction res = eWorld(CSurfInteraction(pHit, DVector2f(u, v), -rayL.dir,
						   dpdu, dpdv, dndu, dndv, rayL.t));
		ecs->addComponent<CSurfInteraction>(interactionRequest.rayCastEntity, std::move(res));
	}, requests);
}


uint32_t SSphere::create(CShape shape,
					   CTransform transform,
					   CSphere sphere)
{
	MEngine& engine = MEngine::getInstance();
	uint32_t hEntity = engine.createEntity();
	shape.type = CShape::Type::Sphere;
	engine.addComponent<CShape>(hEntity, std::move(shape));
	engine.addComponent<CTransform>(hEntity, std::move(transform));
	engine.addComponent<CSphere>(hEntity, std::move(sphere));

	return hEntity;
}

float SSphere::getArea(const CSphere& eSphere)
{
	return 4*PI*eSphere.radius*eSphere.radius;
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

