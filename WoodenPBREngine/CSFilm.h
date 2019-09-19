#pragma once
#include "pch.h"
#include "WoodenMathLibrarry/DBounds.h"
#include "WoodenECS/Job.h"
#include "CSpectrum.h"
#include "CSCamera.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>

WPBR_BEGIN


struct alignas(alignof(DBounds2i)) CFilm
{
	DBounds2i pixelBounds;
	std::vector<uint8_t> rgbOutput;
	std::string outFile;
	uint32_t nTiles;
	uint32_t nProcessedTiles = 0;
};

struct alignas(alignof(DBounds2i)) CFilmTile
{
	DBounds2i pixelBounds;
	std::vector<Spectrum, AllocatorAligned<Spectrum>> pixLiAccumulated;
	std::vector<float, AllocatorAligned<float>> pixFilterWeightAccumulated;
	std::vector<HEntity> samples;
	uint8_t* rgbOutput;
	uint32_t nSamplesPerPixel;

	uint32_t nSamples1DPerCameraSample;
	uint32_t nSamples2DPerCameraSample;
	bool bAccumulated = false;

	uint32_t getPixelIndex(const DPoint2i& p)
	{
		assert(p >= pixelBounds.pMin);
		assert(p <= pixelBounds.pMax);

		DPoint2i d = p - pixelBounds.pMin;

		return d.x() + d.y()*pixelBounds.diagonal().x();
	}
};


class JobGenerateFilmTiles : public JobParallaziblePerCompGroup<CFilm>
{
	void update(WECS* ecs, HEntity hEntity, CFilm& film) final
	{
		uint32_t tileSize = 16;
		uint32_t nPixelsInTile = tileSize * tileSize;
		uint32_t maxConcurrentProcessTiles = std::min(1, film.nTiles-film.nProcessedTiles);
		DPoint2i nTiles = film.pixelBounds.diagonal() / tileSize;
		for (uint32_t i = 0; i < maxConcurrentProcessTiles; i++)
		{
			DPoint2i p = DPoint2i(film.nProcessedTiles % nTiles.x(), film.nProcessedTiles / nTiles.x());

			CFilmTile tile;
			tile.nSamples1DPerCameraSample = 300;
			tile.nSamples2DPerCameraSample = 300;
			tile.nSamplesPerPixel = 16;
			tile.pixLiAccumulated.resize(nPixelsInTile);
			tile.pixFilterWeightAccumulated.resize(nPixelsInTile);
			tile.samples.resize(nPixelsInTile);
			tile.bAccumulated = false;
			tile.pixelBounds = DBounds2i(p*tileSize, (p + DPoint2i(1, 1))*tileSize);
			tile.rgbOutput = film.rgbOutput.data()+tileSize*tileSize*film.nProcessedTiles*3;
			film.nProcessedTiles++;
			HEntity h = ecs->createEntity();
			ecs->addComponent<CFilmTile>(h, std::move(tile));
		}
	}
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
	void updateNStartThreads(uint8_t nWorkThreads) override
	{
		nThreads = nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t sliceSize = (queryComponentsGroup<CFilmTile>().size() - nThreads + 1) / nThreads;
		ComponentsGroupSlice<CFilmTile> tiles =
			queryComponentsGroupSlice<CFilmTile>(Slice(sliceSize*iThread, sliceSize));

		for_each([ecs](HEntity, CFilmTile& tile)
		{
			tile.samples.resize(tile.pixelBounds.area());
			wml::for_each(tile.pixelBounds, [&](const DPoint2i& p, const DPoint2i& ijP, uint32_t iP)
			{
			  HEntity hSample =	ecs->createEntity();
			  CCameraSample camera;
			  camera.pFilm = p;

			  CSamples1D samples1D;
			  samples1D.data.resize(tile.nSamples1DPerCameraSample);

			  CSamples2D samples2D;
			  samples2D.data.resize(tile.nSamples2DPerCameraSample);

			  ecs->addComponent<CSpectrum>(hSample);
			  ecs->addComponent<CCameraSample>(hSample, std::move(camera));
			  ecs->addComponent<CSamples1D>(hSample, std::move(samples1D));
			  ecs->addComponent<CSamples2D>(hSample, std::move(samples2D));

			  tile.nSamplesPerPixel--;
			});
		}, tiles);
	}
};


class JobAccumalateLIFromSamples : public JobParallazible
{
	void updateNStartThreads(uint8_t nWorkThreads) override
	{
		nThreads = nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t sliceSize = (queryComponentsGroup<CFilmTile>().size() - nThreads + 1) / nThreads;
		ComponentsGroupSlice<CFilmTile> samples =
		queryComponentsGroupSlice<CFilmTile>(Slice(sliceSize*iThread, sliceSize));

		for_each([&](HEntity, CFilmTile& tile)
		{
			for (uint32_t i = 0; i < tile.samples.size(); i++)
			{
				const static MitchellFilter filter(4.0, 2, -0.5);

				HEntity hSample = tile.samples[i];
	
				const DPoint2f& samplePos = ecs->getComponent<CCameraSample>(hSample).pFilm;
				const CSpectrum& li = ecs->getComponent<CSpectrum>(hSample);
				
				DPoint2f pDiscrete = samplePos - DVector2f(0.5, 0.5);
				DPoint2i pMin = (DPoint2i)ceil(pDiscrete - filter.radius);
				DPoint2i pMax = (DPoint2i)floor(pDiscrete + filter.radius);
				pMin = maxv(pMin, tile.pixelBounds.pMin);
				pMax = minv(pMax, tile.pixelBounds.pMax);

				for (uint16_t y = pMin.y(); y <= pMax.y(); y++)
				{
					for (uint16_t x = pMin.x(); x <= pMax.x(); x++)
					{
						float weight = filter.evaluate(DPoint2f(x, y) - samplePos);
						int32_t iPixel = tile.getPixelIndex(DPoint2i(x, y));
						tile.pixLiAccumulated[iPixel] += li* weight;
						tile.pixFilterWeightAccumulated[iPixel] += weight;
					}
				}	
			}
		}, samples);
	}
};

class JobOutputFilmTitles : public JobParallazible
{
	void updateNStartThreads(uint8_t nWorkThreads) override
	{
		nThreads = nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t sliceSize = (queryComponentsGroup<CFilmTile>().size() - nThreads + 1) / nThreads;
		ComponentsGroupSlice<CFilmTile> samples =
		queryComponentsGroupSlice<CFilmTile>(Slice(sliceSize*iThread, sliceSize));

		for_each([&](HEntity, CFilmTile& tile)
		{
			wml::for_each(tile.pixelBounds, 
			[&](const DPoint2i& pixel, const DPoint2i& ij, uint32_t i)
			{
				DVector3f rgb= toRGB(tile.pixLiAccumulated[i]/64.0) / tile.pixFilterWeightAccumulated[i];
				tile.rgbOutput[i * 3 + 0] = rgb.x()*255;
				tile.rgbOutput[i * 3 + 1] = rgb.y()*255;
				tile.rgbOutput[i * 3 + 2] = rgb.z()*255;
			});
		}, samples);
	}
};

class JobOutputFilm : public JobParallaziblePerCompGroup<CFilm>
{
	void update(WECS* ecs, HEntity hEntity, CFilm& film) final
	{
		uint32_t tileSize = 16;

		std::vector<uint8_t> data;
		data.resize(film.pixelBounds.area() * 3);
		uint32_t tileRowSize = tileSize * 3;
		uint32_t tileSize = tileSize * tileSize * 3;
		uint32_t tileRowSizeBytes = tileRowSize * 1;
		uint32_t tileSizeBytes = tileSize*1;
		DPoint2i filmSize = film.pixelBounds.diagonal();
		uint8_t* mergeArr = data.data();

		DPoint2i nTiles = film.pixelBounds.diagonal() / tileSize;
		for (uint32_t i = 0; i < film.nTiles; i++)
		{
			DPoint2i iP = DPoint2i(i % nTiles.x(), i/ nTiles.x());

			uint8_t* curTile = film.rgbOutput.data()+i*tileSize;
			for (uint32_t k = 0; k < tileSize; k++)
			{
				uint8_t* dest = mergeArr + filmSize.x()*(tileSize*(iP.y()) + k) + iP.x()*tileSize;
				std::memcpy(dest, curTile, tileRowSizeBytes);
				curTile += tileRowSize;
			}
		}

		FILE *imageFile = fopen(film.outFile.data(), "wb");
		if (imageFile == NULL)
		{
			perror("ERROR: Cannot open output file");
			exit(EXIT_FAILURE);
		}

		fprintf(imageFile, "P6\n");               // P6 filetype
		fprintf(imageFile, "%d %d\n", filmSize.x(), filmSize.y());   // dimensions
		fprintf(imageFile, "255\n");              // Max pixel

		fwrite(mergeArr, 1, data.size(), imageFile);
		fclose(imageFile);
	}

};

WPBR_END

