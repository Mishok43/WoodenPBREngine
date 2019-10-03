#pragma once
#include "pch.h"
#include "CSpectrum.h"

WPBR_BEGIN
struct CSampledBSDFPDF
{
	float p;
	DECL_MANAGED_DENSE_COMP_DATA(CSampledBSDFPDF, 16)
};

struct CSampledBSDFValue : public Spectrum
{
	CSampledBSDFValue() = default;

	CSampledBSDFValue(Spectrum v) :
		Spectrum(std::move(v))
	{
	}

	DECL_MANAGED_DENSE_COMP_DATA(CSampledBSDFValue, 16)
};


struct CBSDFComputeRequest
{
	HEntity h;
	DECL_MANAGED_DENSE_COMP_DATA(CBSDFComputeRequest, 16)
};

struct CBSDFSampleRequest
{
	HEntity h;
	DECL_MANAGED_DENSE_COMP_DATA(CBSDFSampleRequest, 16)
};

//struct alignas(16) CBSDF
//{
//	static constexpr uint32_t maxBXDFs = 4;
//	std::array<HEntity, maxBXDFs> bxdfs;
//	float eta;
//	uint32_t nBXDF = 0;
//
//	uint32_t numComponents(BxDFType flags = BSDF_ALL) const;
//};
//
//Spectrum f(
//	const CSurfaceInteraction& surfInter,
//	const CBSDF& bsdf,
//	const DVector3f& woW, const DVector3f& wiW, 
//	BxDFType flags)
//{
//	const DVector3f& ss = surfInter.shading.dpdu;
//	const DVector3f& ns = surfInter.shading.n;
//	const DVector3f& ts = cross(ns, ss);
//	const DVector3f& ng = surfInter.n;
//	DMatrixf local = DMatrixf(ss, ns, ts);
//
//	DVector3f wo = woW*local;
//	DVector3f wi = wiW* local;
//	bool reflect = dot(wiW, ng)*dot(woW, ng) > 0;
//	for(uint32_t i=0; i < )
//
//	DMatrixf local()
//}
//
//
//

//struct CInteractionSurfEntities: public std::vector<HEntity, AllocatorAligned2<HEntity>>
//{
//	DECL_MANAGED_DENSE_COMP_DATA(CInteractionSurfEntities, 16)
//}; DECL_OUT_COMP_DATA(CInteractionSurfEntities)
//
//class JobBSDFProcessSurfaceInteractions: public JobParallazible
//{
//	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
//	{
//		return min(nWorkThreads, queryComponentsGroup<CInteractionSurfEntities>().size());
//	}
//
//	void update(WECS* ecs, uint8_t iThread) override
//	{
//		uint32_t nBSDFWithInteractions = queryComponentsGroup<CInteractionSurfEntities>().size();
//		uint32_t sliceSize = (nBSDFWithInteractions+nThreads-1) /getNumThreads();
//		ComponentsGroupSlice<CInteractionSurfEntities, CBSDF>  bsdfWithInteractions =
//			queryComponentsGroupSlice<CInteractionSurfEntities, CBSDF>(Slice(iThread*sliceSize, sliceSize));
//
//		for_each([&](HEntity hEntity, const CInteractionSurfEntities& entity, const CBSDF& bsdf)
//		{
//			
//
//
//		}, bsdfWithInteractions);
//	}
//};

// World Matrix

WPBR_END

