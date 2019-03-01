#pragma once
#include "pch.h"

WOODEN_PBR_BEGIN

struct WOptions
{
	std::string inSceneFileName;

	static WOptions processArgumentsToOptions(char* argv[], std::size_t numArguments);
};

WOODEN_PBR_END