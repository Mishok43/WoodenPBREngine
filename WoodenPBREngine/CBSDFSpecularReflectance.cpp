#include "pch.h"
#include "CBSDFSpecularReflectance.h"
#include "MEngine.h"


WPBR_BEGIN

DECL_OUT_COMP_DATA(CBXDFSpecularReflection)


HEntity SBXDFSpecularReflectance::create(float etaI, float etaT, Spectrum R)
{
	MEngine& engine = MEngine::getInstance();
	HEntity hEntity = engine.createEntity();

	engine.addComponent<CBXDFSpecularReflection>(hEntity);
	engine.addComponent<CSpectrumScale>(hEntity, CSpectrumScale(std::move(R)));

	CFresnelDielectric dieletric;
	dieletric.etaI = etaI;
	dieletric.etaT = etaT;
	engine.addComponent<CFresnelDielectric>(hEntity, std::move(dieletric));
	return hEntity;
}

Spectrum SBXDFSpecularReflectance::f_impl(
	const CSpectrumScale& R,
	const CFresnelDielectric& fresnel,
	const DVector3f& wi
)
{
	float cosThetaI = absCosTheta(wi);
	if (cosThetaI == 0)
	{
		return Spectrum(0.);
	}

	float fr = fresnel.f(cosTheta(wi));
	return R*(fr/absCosTheta(wi));
}

Spectrum SBXDFSpecularReflectance::f(
	float& wiPDF
)
{
	wiPDF = 0.0f;
	return Spectrum(0.0);
}


Spectrum SBXDFSpecularReflectance::sample_f(
	const CSpectrumScale& R,
	const CFresnelDielectric& fresnel,
	const DVector3f& wo,
	DVector3f& wi,
	float& wiPDF)
{
	wi = DVector3f(-wo.x(), -wo.y(), wo.z());
	wiPDF = 1.0f;
	return f_impl(R, fresnel, wi);
}

void JobBSDFSpecularReflectionCompute::update(WECS* ecs, HEntity hEntity, CBXDFSpecularReflection& , CBSDFTransform& world, CFresnelDielectric& fresnel,
											  CBSDFComputeRequest& request, CSpectrumScale& R)
{
	const CSampledWI& sampledWI = ecs->getComponent<CSampledWI>(request.h);
	const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(request.h);

	DVector3f wiL = world(sampledWI, INV_TRANFORM);
	CSampledBSDFPDF pdf;
	CSampledBSDFValue bsdf = SBXDFSpecularReflectance::f(pdf.p);
	bsdf = Spectrum(bsdf * absCosTheta(wiL));
	ecs->addComponent<CSampledBSDFValue>(request.h, std::move(bsdf));
	ecs->addComponent<CSampledBSDFPDF>(request.h, std::move(pdf));
}

void JobBSDFSpecularReflectionSample::update(WECS* ecs, HEntity hEntity, CBXDFSpecularReflection&, CBSDFTransform& world, CFresnelDielectric& fresnel,
											 CBSDFSampleRequest& request, CSpectrumScale& R)
{
	const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(request.h);



	CSampledBSDFPDF pdf;
	CSampledWI wi;
	CSampledBSDFValue bsdf = SBXDFSpecularReflectance::sample_f(R, fresnel, world(si.wo), wi, pdf.p);
	bsdf = Spectrum(bsdf*absCosTheta(wi));
	ecs->addComponent<CSampledBSDFValue>(request.h, std::move(bsdf));
	ecs->addComponent<CSampledBSDFPDF>(request.h, std::move(pdf));

	wi = world(wi, INV_TRANFORM);
	ecs->addComponent<CSampledWI>(request.h, std::move(wi));
}



WPBR_END


