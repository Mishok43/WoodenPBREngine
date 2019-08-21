#include "CSCylinder.h"

WPBR_BEGIN

uint32_t SCylinder::create(CShape shape,
					   CTransform transform,
					   CCylinder cylinder)
{
	MEngine& engine = MEngine::getInstance();
	uint32_t hEntity = engine.createEntity();
	shape.type = CShape::Type::Cylinder;
	engine.addComponent<CShape>(hEntity, std::move(shape));
	engine.addComponent<CTransform>(hEntity, std::move(transform));
	engine.addComponent<CCylinder>(hEntity, std::move(cylinder));

	return hEntity;
}

float SCylinder::getArea(const CCylinder& eCylinder)
{
	return 4*PI*eCylinder.radius*eCylinder.radius;
}

bool SCylinder::intersect(const CTransform& eWorld,
					  const CCylinder& eCylinder,
					  const DRayf& rayW)
{
	float phi;
	DPoint3f pHit;

	DVector3f oErr, dErr;
	DRayf rayL = eWorld(rayW, INV_TRANFORM);

	float a = rayL.dir.length2();
	float b = 2 * (dot(rayL.dir, rayL.origin));
	float c = rayL.origin.length2() - eCylinder.radius*eCylinder.radius;

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

bool SCylinder::intersect(const CTransform& eWorld,
					  const CCylinder& eCylinder,
					  HCompR<CShape> hShape,
					  const DRayf& rayW, float& tHit,
					  CInteractionSurface& surfInter)
{
	float phi;
	DPoint3f pHit;

	DVector3f oErr, dErr;
	DRayf rayL = eWorld(rayW, INV_TRANFORM);

	float a = dot<2>(rayL.dir, rayL.dir); // dx*dx + dy*dy
	float b = 2 * (dot<2>(rayL.dir, rayL.origin));
	float c = rayL.origin.length2<2>() - eCylinder.radius*eCylinder.radius;

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
		pHit.x() = 1e-5f*eCylinder.radius;
	}

	float phi = std::atan2(pHit.y(), pHit.x());
	if (phi < 0.0)
	{
		phi += 2 * PI;
	}

	float u = phi / (2 * PI);
	float v = (pHit.z()-eCylinder.zMin)/(eCylinder.zMax-eCylinder.zMin);

	DVector3f dpdu(-pHit.y(), pHit.x(), 0);
	dpdu *= 2 * PI;

	DVector3f dpdv(0, 0, eCylinder.zMax - eCylinder.zMin);


	DVector3f d2Pduu = DVector3f(pHit.x(), pHit.y(), 0)*-2 * PI * 2 * PI;
	DVector3f d2Pduv(0, 0, 0), d2Pdvv(0, 0, 0);

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

	surfInter = eWorld(CInteractionSurface(pHit, DVector3f(), DVector2f(u, v), -rayL.dir,
									dpdu, dpdv, dndu, dndv, rayL.t, hShape));

	return true;
}

DBounds3f SCylinder::getBoundLocal(const CCylinder& eCylinder)
{
	return DBounds3f(DVector3f(-eCylinder.radius, -eCylinder.radius, eCylinder.zMin),
					 DVector3f(eCylinder.radius, eCylinder.radius, eCylinder.zMax));
}

DBounds3f SCylinder::getBoundWorld(const CCylinder& eCylinder,
							   const CTransform& eWorld)
{
	return eWorld(getBoundLocal(eCylinder));
}


WPBR_END

