#pragma once
#include "pch.h"
#include "WoodenMathLibrarry/DPoint.h"

WPBR_BEGIN

struct CCamera
{
	float shutterOpenTime, shutterCloseTime;

	DECL_MANAGED_DENSE_COMP_DATA(CCamera, 1)
}; DECL_OUT_COMP_DATA(CCamera)



struct alignas(alignof(DPoint2f)) CCameraSample
	{
		DPoint2f pFilm;
	};


	WPBR_END