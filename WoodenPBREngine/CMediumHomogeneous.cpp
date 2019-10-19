#include "pch.h"
#include "CMediumHomogeneous.h"
#include "CPhaseFunctionHG.h"
#include "MEngine.h"


WPBR_BEGIN

HEntity SMediumHomogeneous::create(Spectrum sigmaA, Spectrum sigmaS, Spectrum sigmaT, float g)
{
	MEngine& engine = MEngine::getInstance();
	HEntity h = engine.createEntity();


	CMediumHomogeneous medium;
	medium.albedo = std::move(sigmaA);
	medium.scater = std::move(sigmaS);

	CMediumHomogeneousTransmittance tr;
	tr.tr = std::move(sigmaT);

	CPhaseFunctionHG hg;
	hg.g = g;

	engine.addComponent<CMediumHomogeneous>(h, std::move(medium));
	engine.addComponent<CMediumHomogeneousTransmittance>(h, std::move(sigmaT));
	engine.addComponent<CPhaseFunctionHG>(h, std::move(hg));

	return h;
}

Spectrum SMediumHomogeneous::transmittance(const DRayf& ray, float tMax, const Spectrum& sigmaT)
{
	return exp(sigmaT * (min(tMax*ray.dir.length(), INFINITY)*-1));
}

void JobMediumHomogeneousTransmitance::update(WECS* ecs, HEntity hEntity, CMediumHomogeneousTransmittance& transm, CMediumTransmittanceRequests& requests)
{
	for (uint32_t i = 0; i < requests.data.size(); i++)
	{
		HEntity hRequest = requests.data[i];
		const CMediumTransmittanceRequest& request = ecs->getComponent<CMediumTransmittanceRequest>(hRequest);

		CMediumTransmittanceResponse response;
		response.tr = SMediumHomogeneous::transmittance(*request.ray, request.tMax, transm.tr);
		ecs->addComponent<CMediumTransmittanceResponse>(hRequest, std::move(response));
	}

	requests.data.clear();
}


