#include "pch.h"
#include "MEngine.h"
#include "CMedium.h"
#include "CPhaseFunctionHG.h"


WPBR_BEGIN

HEntity SMediumHomogeneous::create(Spectrum sigmaA, Spectrum sigmaS, Spectrum sigmaT, float g)
{
	MEngine& engine = MEngine::getInstance();
	HEntity h = engine.createEntity();


	CMediumHomogeneous medium;
	medium.sigmaA = std::move(sigmaA);
	medium.sigmaS = std::move(sigmaS);

	CMediumTransmittance tr;
	tr.sigma = std::move(sigmaT);

	CPhaseFunctionHG hg;
	hg.g = g;

	engine.addComponent<CMediumHomogeneous>(h, std::move(medium));
	engine.addComponent<CMediumTransmittance>(h, std::move(sigmaT));
	engine.addComponent<CPhaseFunctionHG>(h, std::move(hg));

	return h;
}

Spectrum SMediumHomogeneous::transmittance(const DRayf& ray,float tMax, const Spectrum& sigmaT)
{
	return exp(sigmaT * (min(tMax*ray.dir.length(), INFINITY)*-1));
}

void JobMediumHomogeneousTransmitance::update(WECS* ecs, HEntity hEntity, CMediumTransmittance& transm, CMediumTransmittanceRequests& requests)
{
	for (uint32_t i = 0; i < requests.data.size(); i++)
	{
		HEntity hRequest = requests.data[i];
		const CMediumTransmittanceRequest& request = ecs->getComponent<CMediumTransmittanceRequest>(hRequest);

		CMediumTransmittanceResponse response;
		response.tr = SMediumHomogeneous::transmittance(*request.ray, request.tMax, transm.sigma);
		ecs->addComponent<CMediumTransmittanceResponse>(hRequest, std::move(response));
	}

	requests.data.clear();
}

WPBR_END



