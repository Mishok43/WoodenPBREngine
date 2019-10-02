#pragma once
#include "pch.h"
#include "CFilm.h"
#include "CCamera.h"
#include "CSampler.h"

#include <stdio.h>
#include <stdlib.h>

WPBR_BEGIN




class JobGenerateFilmTiles : public JobParallaziblePerCompGroup<CFilm>
{
	void update(WECS* ecs, HEntity hEntity, CFilm& film) final;
};

struct MitchellFilter
{
public:
	MitchellFilter(float r, float b, float c):
		radius(r), B(b), C(c){ }

	float evaluate(const DPoint2f& p) const
	{
		return evaluate(p.x() / radius)*evaluate(p.y() / radius);
	}

	float evaluate(float x) const
	{
		x = std::abs(2 * x);
		if (x > 1)
			return ((-B - 6 * C) * x*x*x + (6 * B + 30 * C) * x*x +
			(-12 * B - 48 * C) * x + (8 * B + 24 * C)) * (1.f / 6.f);
		else
			return ((12 - 9 * B - 6 * C) * x*x*x +
			(-18 + 12 * B + 6 * C) * x*x +
					(6 - 2 * B)) * (1.f / 6.f);
	}

	float radius,  B,  C;
};


class JobCreateCameraSamples : public JobParallazible
{
	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override;
};


class JobAccumalateLIFromSamples : public JobParallazible
{
	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override;
	void finish(WECS* ecs) override;
};

class JobOutputFilmTitles : public JobParallazible
{
	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override;
	void finish(WECS* ecs) override;
};

class JobOutputFilm : public JobParallaziblePerCompGroup<CFilm>
{
	void update(WECS* ecs, HEntity hEntity, CFilm& film) final;

};

WPBR_END

