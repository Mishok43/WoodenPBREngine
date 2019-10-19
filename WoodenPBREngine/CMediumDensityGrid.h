#pragma once

#include "pch.h"
#include "CMedium.h"

WPBR_BEGIN

class SMediumDensityGrid
{
public:
	HEntity create(const std::string& densityFile, Spectrum albedo, Spectrum scattering);
};


WPBR_END