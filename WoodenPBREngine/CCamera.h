#pragma once
#include "pch.h"
#include "WoodenMathLibrarry/DPoint.h"

WPBR_BEGIN

struct CCamera
{
	float shutterOpenTime, shutterCloseTime;

	DECL_MANAGED_DENSE_COMP_DATA(CCamera, 1)
};



struct alignas(alignof(DPoint2f)) CCameraSample
{
	DPoint2f pFilm;

	DECL_MANAGED_DENSE_COMP_DATA(CCameraSample, 16)
};


WPBR_END