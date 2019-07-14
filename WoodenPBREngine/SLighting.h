#pragma once
#include "pch.h"
#include "WoodenECS/WECS.h"


class CLight;
class CRay;

WPBR_BEGIN

class SLighting
{
	using cmp_type_list = typename wecs::type_list<CLight>;

	void create(size_t hEntity, CLight& light);
public:
	static float lightEmitted(CLight& light, CRay& ray);
	static float lightEmitted(CSufraceInteraction& surf,
							  CRay& ray);

	static float lightSample(CLight& light,
							 CSufraceInteraction& surf,
							 CSampler& sampler,
							 wal::DVector3f& lightToSurf,
							 float& pdf,
							 VisibilityTester& visibility);

	static float funcSample(const DVector3f& wo,
							 CSufraceInteraction& surf,
							 CSampler& sampler,
							 float& pdf,
							 wal::DVector3f& lightToSurf,
							 BxDFType type);

	static CSpectrum lightBSDF(CSufraceInteraction& surf,
							   const CLight& out,
							   const CLight& incident);


	template<typename AllocT>
	static void computeScatteringFunctions(
		CSurfaceInteraction& surfIntersct,
		CRay& ray, AllocT& alloc);
};

WPBR_END