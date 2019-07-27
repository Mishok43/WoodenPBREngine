#pragma once

#include "pch.h"
#include "CTransform.h"

WPBR_BEGIN

struct CTransform;

struct CShape
{
	bool bReverseOrientation;
	bool bTransformSwapsHandedness;

	DECL_UNMANAGED_DENSE_COMP_DATA(CShape, 16)
}; DECL_OUT_COMP_DATA(CShape)

WPBR_END

