#include "pch.h"
#include "CPhaseFunction.h"
#include "MEngine.h"


WPBR_BEGIN


DECL_OUT_COMP_DATA(CPhaseFunctionRequests)
DECL_OUT_COMP_DATA(CPhaseFunctionRequest)
DECL_OUT_COMP_DATA(CPhaseFunctionResponse)
class SPhaseFunctionRequest
{
public:
	HEntity create(DVector3f wi, DVector3f wo)
	{
		MEngine& engine = MEngine::getInstance();
		HEntity h = engine.createEntity();

		CPhaseFunctionRequest request;
		request.wi = wi;
		request.wo = wo;

		engine.addComponent<CPhaseFunctionRequest>(h, std::move(request));
		return h;
	}
};

WPBR_END



