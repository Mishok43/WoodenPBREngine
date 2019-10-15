#pragma once
#include "pch.h"
WPBR_BEGIN



struct CPhaseFunctionResponse
{
	float inScatterPDF;
	DECL_MANAGED_DENSE_COMP_DATA(CPhaseFunctionResponse, 16)
};

struct CPhaseFunctionRequest
{
	DVector3f wi;
	DVector3f wo;

	DECL_MANAGED_DENSE_COMP_DATA(CPhaseFunctionRequest, 1)
};

struct CPhaseFunctionRequests: public std::vector<HEntity>
{
	DECL_MANAGED_DENSE_COMP_DATA(CPhaseFunctionRequests, 2)
};

WPBR_END
