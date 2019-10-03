#include "pch.h"
#include "CTextureMapping.h"

WPBR_BEGIN
DECL_OUT_COMP_DATA(CMapUVRequests)
DECL_OUT_COMP_DATA(CTextureMappedPoint)

void JobMapUVRequestsGenerate::update(WECS* ecs, HEntity hEntity, CSurfaceInteraction& si)
{
	CMapUVRequests& requests = ecs->getComponent<CMapUVRequests>(si.hCollision);
	requests.si.push_back(hEntity);
}
WPBR_END

