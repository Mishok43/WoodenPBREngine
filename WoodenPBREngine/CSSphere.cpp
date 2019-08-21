#include "CSSphere.h"

WPBR_BEGIN

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

bool SSphere::intersect(const CTransform& eWorld,
					  const CSphere& eSphere,
					  const DRayf& rayW)
{
	float phi;
	DPoint3f pHit;

	DVector3f oErr, dErr;
	DRayf rayL = eWorld(rayW, INV_TRANFORM);

	float a = rayL.dir.length2();
	float b = 2 * (dot(rayL.dir, rayL.origin));
	float c = rayL.origin.length2() - eSphere.radius*eSphere.radius;

	float t0, t1;
	if (!HSolver::getRootsQuadraticEquation(a, b, c, t0, t1))
	{
		return false;
	}

	if (t0 > rayL.tMax || t1 <= 0)
		return false;

	float tHit = t0;
	if (tHit <= 0)
	{
		tHit = t1;
		if (tHit > rayL.tMax)
		{
			return false;
		}
	}

	return true;
}

bool SSphere::intersect(const CTransform& eWorld,
					  const CSphere& eSphere,
					  HCompR<CShape> hShape,
					  const DRayf& rayW, float& tHit,
					  CInteractionSurface& surfInter)
{
	float phi;
	DPoint3f pHit;

	DVector3f oErr, dErr;
	DRayf rayL = eWorld(rayW, INV_TRANFORM);

	float a = rayL.dir.length2();
	float b = 2 * (dot(rayL.dir, rayL.origin));
	float c = rayL.origin.length2() - eSphere.radius*eSphere.radius;

	float t0, t1;
	if (!HSolver::getRootsQuadraticEquation(a, b, c, t0, t1))
	{
		return false;
	}

	if (t0 > rayL.tMax || t1 <= 0)
		return false;

	tHit = t0;
	if (tHit <= 0)
	{
		tHit = t1;
		if (tHit > rayL.tMax)
		{
			return false;
		}
	}

	DPoint3f pHit = rayL(tHit);

	if (pHit.x() == 0 && pHit.y() == 0)
	{
		pHit.x() = 1e-5f*eSphere.radius;
	}

	float phi = std::atan2(pHit.y(), pHit.x());
	if (phi < 0.0)
	{
		phi += 2 * PI;
	}

	float u = phi / 2 * PI;
	float theta = std::acos(pHit.z() / eSphere.radius);
	float v = theta / PI;

	float invZRadius = 1.0/std::sqrt(pHit.x()*pHit.x() + pHit.y()*pHit.y());
	float phiCos = pHit.x()*invZRadius;
	float phiSin = pHit.y()*invZRadius;

	DVector3f dpdu = DVector3f(-pHit.y(), pHit.x(), 0.0);
	dpdu *= 2 * PI;

	DVector3f dpdv = DVector3f(pHit.z()*phiCos, pHit.z()*phiSin, -eSphere.radius*sin(theta));
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
	DNormal3f dndu = DNormal3f(dpdu*((f * F - e * G) * invEGF2)+
				dpdv*((e * F - f * E) * invEGF2));
	DNormal3f dndv = DNormal3f(dpdu*((g * F - f * G) * invEGF2) +
							   dpdv*((f * F - g * E) * invEGF2));

	//const HComp<CShape> h = hShape;

	surfInter = eWorld(CInteractionSurface(pHit, DVector3f(), DVector2f(u, v), -rayL.dir,
									dpdu, dpdv, dndu, dndv, rayL.t, hShape));

	return true;
}

DBounds3f SSphere::getBoundLocal(const CSphere& eSphere)
{
	return DBounds3f(DVector3f(-eSphere.radius, -eSphere.radius, -eSphere.radius),
					 DVector3f(eSphere.radius, eSphere.radius, eSphere.radius));
}

DBounds3f SSphere::getBoundWorld(const CSphere& eSphere,
							   const CTransform& eWorld)
{
	return eWorld(getBoundLocal(eSphere));
}


WPBR_END

