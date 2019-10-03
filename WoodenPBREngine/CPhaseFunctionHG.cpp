#include "pch.h"
#include "CPhaseFunctionHG.h"


WPBR_BEGIN

class SPhaseFunctionHG
{
public:
	static float p(const CPhaseFunctionHG& hg, const DVector3f& wo, const DVector3f& wi)
	{
		return p(hg, dot(wo, wi));
	}

	static float p(const CPhaseFunctionHG& hg, const float cosTheta)
	{
		float denom = 1 + hg.g*hg.g + 2 * hg.g*cosTheta;
		return (1.0 / 4 * PI)*(1.0 - hg.g*hg.g) / (denom*std::sqrt(denom));
	}
};


void JobPhaseFunctionHDCompute::update(WECS* ecs, HEntity hEntity, CPhaseFunctionHG& hg, CPhaseFunctionRequests& requests)
{
	for (uint32_t i = 0; i < requests.size(); i++)
	{
		HEntity hRequest = requests[i]; 

		CPhaseFunctionRequest& request = ecs->getComponent<CPhaseFunctionRequest>(hRequest);
		CPhaseFunctionResponse res;
		res.inScatterPDF = SPhaseFunctionHG::p(hg, request.wi, request.wo);
		ecs->addComponent<CPhaseFunctionResponse>(hRequest, std::move(res));
	}

	requests.clear();
}

WPBR_END


