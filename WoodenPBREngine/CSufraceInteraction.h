#pragma once

#include "pch.h"
#include "CRay.h"

WPBR_BEGIN

class CSufraceInteraction
{
	CRay ray;
	class CPrimitive* primitive;
	bool intersection;
};


WPBR_END

