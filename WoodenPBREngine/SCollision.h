#pragma once
#include "pch.h"
#include "CSufraceInteraction.h"
#include "CPrimitive.h"

WPBR_BEGIN

class SCollision
{
public:
	using cmp_type_list = typename wecs::type_list<CSufraceInteraction>;

	void create(size_t hEntity, CSufraceInteraction& c1)
	{
		
	}
};

WPBR_END


