#pragma once
#include "pch.h"
//#include "MEngine.h"
//
//WPBR_BEGIN
//
//struct CPhaseFunction
//{	
//	enum class Type
//	{
//		HG,
//		COUNT
//	};
//
//	Type type = Type::COUNT;
//	DECL_MANAGED_DENSE_COMP_DATA(CPhaseFunction, 4)
//}; DECL_OUT_COMP_DATA(CPhaseFunction)
//
//struct CPhaseFunctionHG
//{
//	float g;
//};
//
//class SPhaseFunction
//{
//public:
//	virtual float p(const HEntity& hEntity, HCompUniversal<CompDummy>& hComp, const DVector3f& wo, const DVector3f& wi) = 0;
//};
//
//class SPhaseFunctionHG: public SPhaseFunction
//{
//public:
//	static HEntity create(CPhaseFunctionHG hg)
//	{
//		MEngine& engine = MEngine::getInstance();
//		uint32_t hEntity = engine.createEntity();
//		
//		CPhaseFunction phaseFunc;
//		phaseFunc.type = CPhaseFunction::Type::HG;
//
//		engine.addComponent<CPhaseFunction>(hEntity, std::move(phaseFunc));
//		engine.addComponent<CPhaseFunctionHG>(hEntity, std::move(hg));
//
//		return hEntity;
//	}
//
//	virtual float p(const HEntity& hEntity, HCompUniversal<CompDummy>& hCompDummy, const DVector3f& wo, const DVector3f& wi) override
//	{
//		const HCompUniversal<CPhaseFunctionHG>& hComp = (hCompDummy) ? hCompDummy : 
//			HCompUniversal<CPhaseFunctionHG>(hEntity.getComponentHandle<CPhaseFunctionHG>());
//
//		const CPhaseFunctionHG& hg = hComp.get<CPhaseFunctionHG>();
//		p(hg, wo, wi);
//	}
//
//	static float p(const CPhaseFunctionHG& hg, const DVector3f& wo, const DVector3f& wi)
//	{
//		return p(hg, dot(wo, wi));
//	}
//
//	static float p(const CPhaseFunctionHG& hg, const float cosTheta)
//	{
//		float denom = 1 + hg.g*hg.g + 2 * hg.g*cosTheta;
//		return (1.0 / 4 * PI)*(1.0 - hg.g*hg.g) / (denom*std::sqrt(denom));
//	}
//
//};
//
//WPBR_END