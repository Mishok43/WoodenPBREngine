#pragma once
#include "pch.h"

WPBR_BEGIN

struct CShape
{
	enum class Type
	{
		Sphere,
		Cylinder,
		Triangle,
		SIZE
	};

	CShape(bool bReverseOrientation,
		   bool bTransformSwapsHandedness) :
		bReverseOrientation(bReverseOrientation),
		bTransformSwapsHandedness(bTransformSwapsHandedness)
	{
	}

	bool bReverseOrientation;
	bool bTransformSwapsHandedness;
	Type type;

	DECL_UNMANAGED_DENSE_COMP_DATA(CShape, 16)
}; DECL_OUT_COMP_DATA(CShape)


WPBR_END