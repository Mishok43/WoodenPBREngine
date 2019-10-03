#include "pch.h"
#include "CPhaseFunction.h"
#include "MEngine.h"


WPBR_BEGIN

DECL_OUT_COMP_DATA(CPhaseFunctionRequestWI)
DECL_OUT_COMP_DATA(CPhaseFunctionRequestWO)
DECL_OUT_COMP_DATA(CPhaseFunctionRequests)

class SPhaseFunctionRequest
{
public:
	HEntity create(DVector3f wi, DVector3f wo)
	{
		MEngine& engine = MEngine::getInstance();
		HEntity h = engine.createEntity();
		engine.addComponent<CPhaseFunctionRequestWI>(h, CPhaseFunctionRequestWI(std::move(wi)));
		engine.addComponent<CPhaseFunctionRequestWO>(h, CPhaseFunctionRequestWO(std::move(wo)));
		return h;
	}
};

WPBR_END



