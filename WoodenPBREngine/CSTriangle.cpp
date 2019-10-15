#include "pch.h"
#include "CSTriangle.h" 
WPBR_BEGIN

DECL_OUT_COMP_DATA(CInteractionTriangle)
DECL_OUT_COMP_DATA(CTriangle)


void JobProcessTriangleInteractionRequests::update(WECS* ecs, HEntity hEntity, CInteractionTriangle& interactionTriangle, CInteractionRequest& interactionRequest)
{
	const CRayCast& rayCast = ecs->getComponent<CRayCast>(interactionRequest.rayCastEntity);
	const DRayf& ray = rayCast.ray;

	const CTriangle& triangle = ecs->getComponent<CTriangle>(interactionRequest.hShape);
	const CTriangleMesh& triangleMesh = ecs->getComponent<CTriangleMesh>(triangle.hMesh);

	std::array<uint32_t, 3> iVertex = {
		triangleMesh.iVertices[triangle.index * 3 + 0],
		triangleMesh.iVertices[triangle.index * 3 + 1],
		triangleMesh.iVertices[triangle.index * 3 + 2]
	};

	std::array<DPoint3f, 3> vPositions = {
		triangleMesh.positions[iVertex[0]],
		triangleMesh.positions[iVertex[1]],
		triangleMesh.positions[iVertex[2]]
	};


	DVector3u iPermutation;
	iPermutation.z() = maxDimension(abs(ray.dir));
	iPermutation.x() = (iPermutation.z() == 2) ? 0 : iPermutation.z() + 1;
	iPermutation.y() = (iPermutation.z() == 2) ? 1 : iPermutation.z() + 2;
	DVector3f d = permute(ray.dir, iPermutation);

	DVector3f shear;
	shear.x() = -d.x();
	shear.y() = -d.y();
	shear.z() = 1.0f;
	shear /= d.z();

	std::array<DVector3f, 3> posT;
	for (uint8_t i = 0; i < vPositions.size(); i++)
	{
		DVector3f p = vPositions[i] - ray.origin;
		p = permute(p, iPermutation);
		p += DVector3f(shear.x(), shear.y(), 0.0f)*p.z();
		posT[i] = p;
	}

	std::array<DVector3f, 3> posTSoA;
	// convert from AoS to SoA for faster computing
	for (uint8_t i = 0; i < 3; i++)
	{
		posTSoA[i] = DVector3f(posT[0][i], posT[1][i], posT[2][i]);
	}

	DVector3f y0 = DVector3f(posTSoA[1].permute<0b00001001>()); // 1200
	DVector3f x0 = DVector3f(posTSoA[0].permute<0b00001001>());

	DVector3f res = posTSoA[0] * y0 - x0 * posTSoA[1];

	if ((res.x() < 0 || res.y() < 0 || res.z() < 0) && (res.x() > 0 || res.y() > 0 || res.z() > 0))
	{
		interactionRequest.tHitResult = 0.0f;
		return;
	}

	float det = res.x() + res.y() + res.z();
	if (det == 0)
	{
		interactionRequest.tHitResult = 0.0f;
		return;
	}

	res = res.permute<0b00001001>();

	float tScale = dot(res, posTSoA[2]* shear.z());
	if (det < 0.0 && (tScale >= 0))
	{
		interactionRequest.tHitResult = 0.0f;
		return;
	}

	if (det > 0.0 && (tScale <= 0))
	{
		interactionRequest.tHitResult = 0.0f;
		return;
	}

	float invDet = 1.0 / det;
	DPoint3f barycentric = DPoint3f(res * invDet);
	float tHit = tScale * invDet;

	interactionRequest.tHitResult = tHit;
	interactionTriangle.barycentric = barycentric;
	interactionTriangle.tHit = tHit;
}


CTextureMappedPoint STriangle::mapUV(const CSurfaceInteraction& si)
{
	return TextureMappingUV::map(si);
}

void JobUpdateBoundsAndCentroidTriangle::update(WECS* ecs, uint8_t iThread) 
{
	uint32_t nCollisions = queryComponentsGroup<CTriangle>().size<CTriangle>();
	uint32_t sliceSize = (nCollisions + getNumThreads() - 1) / getNumThreads();
	uint32_t iStart = iThread * sliceSize;

	ComponentsGroupSlice<CTriangle, CBounds, CCentroid> triangles =
		queryComponentsGroupSlice<CTriangle, CBounds, CCentroid>(Slice(iStart, sliceSize));

	for_each([ecs](HEntity hEntity, const CTriangle& eTriangle,
			 CBounds& eBounds, CCentroid& eCentroid)
	{
		const CTriangleMesh& triangleMesh = ecs->getComponent<CTriangleMesh>(eTriangle.hMesh);

		std::array<uint32_t, 3> iVertex =
		{
			triangleMesh.iVertices[eTriangle.index * 3],
			triangleMesh.iVertices[eTriangle.index * 3 + 1],
			triangleMesh.iVertices[eTriangle.index * 3 + 2]
		};

		DBounds3f bounds = DBounds3f(triangleMesh.positions[iVertex[0]],
									 triangleMesh.positions[iVertex[1]]);
		eBounds = DBounds3f(bounds, triangleMesh.positions[iVertex[2]]);
		eCentroid = CCentroid(eBounds);
	}, triangles);
}

void JobProcessTriangleFullInteractionRequests::finish(WECS* ecs)
{
	ecs->clearComponents<CInteractionTriangle>();
}

void JobProcessTriangleFullInteractionRequests::update(WECS* ecs, HEntity hEntity, CFullInteractionRequest&,
													   CInteractionTriangle& interactionTriangle,
													   CInteractionRequest& interactionRequest)
{
	const CRayCast& rayCast = ecs->getComponent<CRayCast>(interactionRequest.rayCastEntity);
	const DRayf& ray = rayCast.ray;

	const CTriangle& triangle = ecs->getComponent<CTriangle>(interactionRequest.hShape);
	const CTriangleMesh& triangleMesh = ecs->getComponent<CTriangleMesh>(triangle.hMesh);
	DPoint3f& barycentric = interactionTriangle.barycentric;
	float& tHit = interactionTriangle.tHit;


	std::array<DVector2f, 3> vUVs;
	std::array<uint32_t, 3> iVertex = {
		triangleMesh.iVertices[triangle.index * 3 + 0],
		triangleMesh.iVertices[triangle.index * 3 + 1],
		triangleMesh.iVertices[triangle.index * 3 + 2]
	};

	std::array<DPoint3f, 3> vPositions = {
		triangleMesh.positions[iVertex[0]],
		triangleMesh.positions[iVertex[1]],
		triangleMesh.positions[iVertex[2]]
	};


	if (triangleMesh.uvs.size() == 0)
	{
		vUVs[0] = DPoint2f(0.0, 0.0);
		vUVs[1] = DPoint2f(1.0, 0.0);
		vUVs[2] = DPoint2f(1.0, 1.0);
	}
	else
	{
		vUVs[0] = triangleMesh.uvs[iVertex[0]];
		vUVs[1] = triangleMesh.uvs[iVertex[1]];
		vUVs[2] = triangleMesh.uvs[iVertex[2]];
	}


	DVector3f dp02 = vPositions[0] - vPositions[2];
	DVector3f dp12 = vPositions[1] - vPositions[2];

	DVector3f dpdu, dpdv;

	DVector2f duv02 = vUVs[0] - vUVs[2];
	DVector2f duv12 = vUVs[1] - vUVs[2];

	{
		float det = duv02[0] * duv12[1] - duv02[1] * duv12[0];
		if (det == 0)
		{
			vUVs[0] = DPoint2f(0.0, 0.0)*0.001;
			vUVs[1] = DPoint2f(1.0, 0.0)*0.001;
			vUVs[2] = DPoint2f(1.0, 1.0)*0.001;

			duv02 = vUVs[0] - vUVs[2];
			duv12 = vUVs[1] - vUVs[2];

			det = duv02[0] * duv12[1] - duv02[1] * duv12[0];

		}
		
			float detInv = 1 / det;
			dpdu = (dp02*duv12[1] - dp12 * duv02[1]) * detInv;
			dpdv = (-dp02 * duv12[0] + dp12 * duv02[0]) * detInv;
		
	}

	DPoint3f pHit =
		vPositions[0] * barycentric[0] +
		vPositions[1] * barycentric[1] +
		vPositions[2] * barycentric[2];


	DPoint2f uvHit =
		vUVs[0] * barycentric[0] +
		vUVs[1] * barycentric[1] +
		vUVs[2] * barycentric[2];

	float l = dpdu.length2();


	CSurfaceInteraction res = CSurfaceInteraction(pHit, uvHit, -ray.dir, dpdu, dpdv,
												  DVector3f(0.0f, 0.0f, 0.0f),
												  DVector3f(0.0f, 0.0f, 0.0f),
												  tHit, interactionRequest.hShape);

	res.n = res.shading.n = DNormal3f(normalize(cross(dp02, dp12)));


	if (!triangleMesh.normals.empty() && !triangleMesh.tangents.empty())
	{
		std::array<DPoint3f, 3> vNormals = {
			triangleMesh.normals[iVertex[0]],
			triangleMesh.normals[iVertex[1]],
			triangleMesh.normals[iVertex[2]]
		};


		DNormal3f ns =
			vNormals[0] * barycentric[0] +
			vNormals[1] * barycentric[1] +
			vNormals[2] * barycentric[2];
		if (!ns.bIsAllZero())
		{
			ns = normalize(ns);
		}
		else
		{
			ns = res.n;
		}

		std::array<DPoint3f, 3> vTangents = {
			triangleMesh.tangents[iVertex[0]],
			triangleMesh.tangents[iVertex[1]],
			triangleMesh.tangents[iVertex[2]]
		};

		DVector3f ss =
			vTangents[0] * barycentric[0] +
			vTangents[1] * barycentric[1] +
			vTangents[2] * barycentric[2];
		if (!ss.bIsAllZero())
		{
			ss = normalize(ss);
		}
		else
		{
			ss = normalize(res.dpdu);
		}

		DVector3f ts = cross(ss, ns);
		if (ts.length2() > 0)
		{
			ts = normalize(ts);
			ss = cross(ts, ns);
		}

		DVector3f dn02 = vNormals[0] - vNormals[2];
		DVector3f dn12 = vNormals[1] - vNormals[2];

		DVector3f dndu, dndv;
		{
			float det = duv02[0] * duv12[1] - duv02[1] * duv12[0];
			if (det == 0)
			{
				/*Handle zero determinant for triangle partial derivative matrix 164*/
			}
			else
			{
				float detInv = 1 / det;
				dndu = (dn02*duv12[1] - dn12 * duv02[1]) * detInv;
				dndv = (-dn02 * duv12[0] + dn12 * duv02[0]) * detInv;
			}
		}

		res.setShadingGeometry(ss, ts, dndu, dndv, true);
	}

	ecs->addComponent<CSurfaceInteraction>(interactionRequest.rayCastEntity, std::move(res));
}

void JobTriangleProcessMapUVRequests::update(WECS* ecs, HEntity hEntity, CTriangle& triangle, CMapUVRequests& requests)
{
	for (uint32_t i = 0; i < requests.si.size(); i++)
	{
		const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(requests.si[i]);
		CTextureMappedPoint mp = STriangle::mapUV(si);
		ecs->addComponent<CTextureMappedPoint>(requests.si[i], mp);
	}
	requests.si.clear();
}

WPBR_END