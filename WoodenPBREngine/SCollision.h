#pragma once
#include "pch.h"
#include "CSufraceInteraction.h"
#include "CPrimitive.h"

WPBR_BEGIN

class CRay;


class SCollision
{
public:
	using cmp_type_list = typename wecs::type_list<>;



	static CSufraceInteraction raycast(CRay& ray, size_t hPrimitive);

};

WPBR_END


