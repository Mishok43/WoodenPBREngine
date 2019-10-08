#include "pch.h"
#include "CBSDF.h"

WPBR_BEGIN
DECL_OUT_COMP_DATA(CSampledBSDFPDF)
DECL_OUT_COMP_DATA(CSampledBSDFValue)
DECL_OUT_COMP_DATA(CBSDFComputeRequest)
DECL_OUT_COMP_DATA(CBSDFSampleRequest)
DECL_OUT_COMP_DATA(CBSDFTransform)
DECL_OUT_COMP_DATA(CSampledBSDF)

void JobBSDFComputeTransform::update(WECS* ecs, HEntity hEntity, CSampledBSDF& sampledBSDF, CSurfaceInteraction& si)
{
	CTransform world;

	DVector3f ss = normalize(si.shading.dpdu);
	const DVector3f& ns = si.shading.n;
	DVector3f ts = cross(ns, ss);
	world.m() = DMatrixf(DVector4f(ss), DVector4f(ts), DVector4f(ns), DVector4f(0.0f, 0.0f, 0.0f, 1.0f));
	world.mInv() = transpose(world.m());
	ecs->addComponent<CBSDFTransform>(sampledBSDF.h, CBSDFTransform(std::move(world)));
}
WPBR_END

