#include "pch.h"
//#pragma once
//
//#include "pch.h"
//#include "CSufraceInteraction.h"
//#include "CPrimitive.h"
//#include "CScene.h"
//#include "CCamera.h"
//#include "CSampler.h"
//#include "WoodenAllocators/AllocatorLinear.h"
//#include "WoodenMathLibrarry/DVector.h"
//
//#include "SLighting.h"
//
//WPBR_BEGIN
//template<typename AllocT = wal::AllocatorLinear>
//class SIntegratorWhitted
//{
//	using cmp_type_list = typename wecs::type_list<CScene, CCamera, CSampler>;
//
//	SIntegratorWhitted(uint32_t maxDepth):
//		maxDepth(maxDepth)
//	{}
//
//	void preprocess(CScene& scene, CSampler& sampler);
//
//	
//	CSpectrum lightIn(const CRayDifferntial& ray,
//				 CSampler& sampler,
//				 AllocT& alloc,
//				 uint32_t depth = 0);
//
//	
//	CSpectrum SIntegratorWhitted::specularReflect(
//		const CRayDifferential& ray,
//		const CSurfaceInteraction& surfIntersct,
//		CSampler& sampler,
//		AllocT& alloc,
//		uint32_t depth) const;
//
//	void renderTile(DVector2u&& tileBounds,
//					CSampler&& tileSampler,
//					CFileTile&& tileFilm);
//
//	void updateECS(wecs::WECS& _engine,
//				   CScene& scene, 
//				   CCamera& camera, 
//				   CSampler& sampler);
//
//protected:
//	CScene* cScene;
//	CCamera* cCamera;
//	wecs::WECS* engine;
//	const uint32_t maxDepth;
//};
//
//WPBR_END
//
//
//
