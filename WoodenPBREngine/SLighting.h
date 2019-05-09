#pragma once
#include "pch.h"
#include "WoodenECS/WECS.h"
#include "CLight.h"

WPBR_BEGIN

class SLighting
{
	using cmp_type_list = typename wecs::type_list<CLight>;

	void create(size_t hEntity, CLight& light);

};

WPBR_END