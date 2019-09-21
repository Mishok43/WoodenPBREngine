#include "pch.h"
#include "CSTriangle.h" 
WPBR_BEGIN


void JobProcessTriangleFullInteractionRequests::update(WECS* ecs, uint8_t iThread)
{
	uint32_t nCollisions = queryComponentsGroup<CInteractionTriangle>().size<CInteractionTriangle>();
	uint32_t sliceSize = (nCollisions + getNumThreads()-1) /getNumThreads();
	uint32_t iStart = iThread * sliceSize;

	ComponentsGroupSlice<CFullInteractionRequest, CInteractionTriangle, CInteractionRequest> requests =
		queryComponentsGroupSlice<CFullInteractionRequest, CInteractionTriangle, CInteractionRequest>(Slice(iStart, sliceSize));

	for_each([ecs](HEntity hEntity,
			 CFullInteractionRequest&,
			 CInteractionTriangle& interactionTriangle,
			 CInteractionRequest& interactionRequest)
	{
		const CRayCast& rayCast = ecs->getComponent<CRayCast>(interactionRequest.rayCastEntity);
		const DRayf& ray = rayCast.ray;

		const CTriangle& triangle = ecs->getComponent<CTriangle>(interactionTriangle.hTriangle);
		const CTriangleMesh& triangleMesh = ecs->getComponent<CTriangleMesh>(interactionTriangle.hTriangle);
		std::array<DVector3f, 3>& posT = interactionTriangle.posT;
		DPoint3f& barycentric = interactionTriangle.barycentric;
		float& tHit = interactionTriangle.tHit;


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


		std::array<DPoint2f, 3> vUVs;
		if (!triangleMesh.uvs.empty())
		{
			vUVs = {
				DPoint2f(0.0, 0.0),
				DPoint2f(1.0, 0.0),
				DPoint2f(1.0, 1.0)
			};
		}
		else
		{
			vUVs = {
				triangleMesh.uvs[iVertex[0]],
				triangleMesh.uvs[iVertex[1]],
				triangleMesh.uvs[iVertex[2]]
			};
		}


		DVector3f dp02 = posT[0] - posT[2];
		DVector3f dp12 = posT[1] - posT[2];

		DVector3f dpdu, dpdv;

		DVector2f duv02 = vUVs[0] - vUVs[2];
		DVector2f duv12 = vUVs[1] - vUVs[2];

		{
			float det = duv02[0] * duv12[1] - duv02[1] * duv12[0];
			if (det == 0)
			{
				/*Handle zero determinant for triangle partial derivative matrix 164*/
			}
			else
			{
				float detInv = 1 / det;
				dpdu = (dp02*duv12[1] - dp12 * duv02[1]) * detInv;
				dpdv = (-dp02 * duv12[0] + dp12 * duv02[0]) * detInv;
			}
		}

		DPoint3f pHit =
			vPositions[0] * barycentric[0] +
			vPositions[1] * barycentric[1] +
			vPositions[2] * barycentric[2];


		DPoint2f uvHit =
			vUVs[0] * barycentric[0] +
			vUVs[1] * barycentric[1] +
			vUVs[2] * barycentric[2];

		CSurfaceInteraction res = CSurfaceInteraction(pHit, uvHit, -ray.dir, dpdu, dpdv,
										DNormal3f(0.0f, 0.0f, 0.0f),
										DNormal3f(0.0f, 0.0f, 0.0f), tHit);

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

	}, requests);
}

void JobProcessTriangleInteractionRequests::update(WECS* ecs, uint8_t iThread)
{
	uint32_t nCollisions = queryComponentsGroup<CInteractionTriangle>().size<CInteractionTriangle>();
	uint32_t sliceSize = (nCollisions + getNumThreads()-1) /getNumThreads();
	uint32_t iStart = iThread * sliceSize;

	ComponentsGroupSlice<CInteractionTriangle, CInteractionRequest> requests =
		queryComponentsGroupSlice<CInteractionTriangle, CInteractionRequest>(Slice(iStart, sliceSize));

	for_each([ecs](HEntity hEntity,
			 CInteractionTriangle& interactionTriangle,
			 CInteractionRequest& interactionRequest)
	{
		const CRayCast& rayCast = ecs->getComponent<CRayCast>(interactionRequest.rayCastEntity);
		const DRayf& ray = rayCast.ray;

		const CTriangle& triangle = ecs->getComponent<CTriangle>(interactionTriangle.hTriangle);
		const CTriangleMesh& triangleMesh = ecs->getComponent<CTriangleMesh>(interactionTriangle.hTriangle);

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
		iPermutation.y() = (iPermutation.z() == 1) ? 0 : iPermutation.z() + 2;
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
			p = permute(posT[i], iPermutation);
			p += p * shear;
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

		float tScale = dot(res, posTSoA[2]);
		if (det < 0.0 && (tScale >= 0 || tScale < ray.tMax*det))
		{
			interactionRequest.tHitResult = 0.0f;
			return;
		}

		if (det > 0.0 && (tScale <= 0 || tScale > ray.tMax*det))
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
		interactionTriangle.posT = std::move(posT);
	}, requests);
}


CTextureMappedPoint STriangle::mapUV(const CSurfaceInteraction& si)
{
	return TextureMappingUV::map(si);
}

WPBR_END