#include "pch.h"
#include "CSFilm.h"
#include "lodepng.h"
#include <iostream>
#include "Settings.h"
#include "CTexture.h"
WPBR_BEGIN


void JobGenerateFilmTiles::update(WECS* ecs, HEntity hEntity, CFilm& film)
{
	uint32_t tileSize = 32;
	uint32_t nPixelsInTile = tileSize * tileSize;
	uint32_t nAbsTiles = film.resolution.x() / tileSize * film.resolution.y() / tileSize;
	uint32_t maxConcurrentProcessTiles = min(1, nAbsTiles - film.nProcessedTiles);
	DPoint2i nTiles = film.resolution / tileSize;
	for (uint32_t i = 0; i < maxConcurrentProcessTiles; i++)
	{
		DPoint2i p = DPoint2i(film.nProcessedTiles % nTiles.x(), film.nProcessedTiles / nTiles.x());

		CFilmTile tile;
		tile.nSamples1DPerCameraSample = 16+ NSAMPLERS_PER_PIXEL;
		tile.nSamples2DPerCameraSample = 16+ NSAMPLERS_PER_PIXEL *4;
		tile.nSamplesPerPixel = 1;
		tile.pixLiAccumulated.resize(nPixelsInTile);
		tile.pixFilterWeightAccumulated.resize(nPixelsInTile);
		tile.samples.reserve(nPixelsInTile);
		tile.bAccumulated = false;
		tile.pixelBounds = DBounds2i(p*tileSize, (p + DPoint2i(1, 1))*tileSize - DPoint2i(1, 1));
		tile.rgbOutput = film.rgbOutput.data() + tileSize * tileSize*film.nProcessedTiles * 3;
		film.nProcessedTiles++;
		HEntity h = ecs->createEntity();
		ecs->addComponent<CFilmTile>(h, std::move(tile));
	}
}




void JobCreateCameraSamples::update(WECS* ecs, uint8_t iThread)
{
	uint32_t sliceSize = (queryComponentsGroup<CFilmTile>().size() - nThreads + 1) / getNumThreads();
	ComponentsGroupSlice<CFilmTile> tiles =
		queryComponentsGroupSlice<CFilmTile>(Slice(sliceSize*iThread, sliceSize));

	for_each([ecs](HEntity, CFilmTile& tile)
	{
		tile.samples.clear();
		wml::for_each(tile.pixelBounds, [&](const DPoint2i& p, const DPoint2i& ijP, uint32_t iP)
		{
			HEntity hSample = ecs->createEntity();

			tile.samples.push_back(hSample);
			CCameraRasterPoint camera;
			camera.p = (DPoint2f)p;

			CSamples1D samples1D;
			samples1D.data.resize(tile.nSamples1DPerCameraSample);

			CSamples2D samples2D;
			samples2D.data.resize(tile.nSamples2DPerCameraSample);

			ecs->addComponent<CCameraRasterPoint>(hSample, std::move(camera));
			ecs->addComponent<CSamples1D>(hSample, std::move(samples1D));
			ecs->addComponent<CSamples2D>(hSample, std::move(samples2D));
		});
	}, tiles);
}

void JobAccumalateLIFromSamples::update(WECS* ecs, uint8_t iThread) 
{
	uint32_t sliceSize = (queryComponentsGroup<CFilmTile>().size() - nThreads + 1) / getNumThreads();
	ComponentsGroup<CFilmTile> samples =
		queryComponentsGroup<CFilmTile>();

	for_each([&](HEntity, CFilmTile& tile)
	{
		for (uint32_t i = 0; i < tile.samples.size(); i++)
		{
			const static MitchellFilter filter(1.65f, 1.0/3.0f, 1.0/3.0f);

			HEntity hSample = tile.samples[i];

			const DPoint2f& samplePos = ecs->getComponent<CCameraSample>(hSample).pFilm;
			const CSpectrum& li = ecs->getComponent<CSpectrum>(hSample);
			/*if (li.isBlack())
			{

				continue;
			}*/
				
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
					

					if (!li.isBlack())
					{
						tile.pixLiAccumulated[iPixel] += li * weight;
					}
					else
					{
					}

					tile.pixFilterWeightAccumulated[iPixel] += weight;
				}
			}
		}

		
	}, samples);
}

void JobAccumalateLIFromSamples::finish(WECS* ecs)
{
	ecs->clearComponents<CSpectrum>();
	ecs->clearComponents<CCameraSample>();
}
float gammaCorrect(float value)
{
	if (value <= 0.0031308f)
	{
		return 12.98f*value;
	}
	else
	{
		return 1.055f * std::pow(value, (float)(1.f / 2.4f)) - 0.055f;
	}
}

void JobOutputFilmTitles::update(WECS* ecs, uint8_t iThread)
{
	uint32_t sliceSize = (queryComponentsGroup<CFilmTile>().size() - nThreads + 1) / getNumThreads();
	ComponentsGroupSlice<CFilmTile> samples =
		queryComponentsGroupSlice<CFilmTile>(Slice(sliceSize*iThread, sliceSize));

	for_each([&](HEntity, CFilmTile& tile)
	{
		wml::for_each(tile.pixelBounds,
					  [&](const DPoint2i& pixel, const DPoint2i& ij, uint32_t i)
		{
			if (tile.pixLiAccumulated[i].isBlack())
			{
				tile.rgbOutput[i * 3 + 0] = 0;
				tile.rgbOutput[i * 3 + 1] = 0;
				tile.rgbOutput[i * 3 + 2] = 0;
				return;
			}
			DVector3f rgb = toRGB(tile.pixLiAccumulated[i]) /(tile.pixFilterWeightAccumulated[i]* 5000000.f);
			rgb = wml::clamp(rgb, 0.0, 1.0f);
			rgb.x() = gammaCorrect(rgb.x());
			rgb.y() = gammaCorrect(rgb.y());
			rgb.z() = gammaCorrect(rgb.z());
			rgb *= 255.0f;
			tile.rgbOutput[i * 3 + 0] = rgb.x();
			tile.rgbOutput[i * 3 + 1] = rgb.y();
			tile.rgbOutput[i * 3 + 2] = rgb.z();
		});
	}, samples);
}

void JobOutputFilmTitles::finish(WECS* ecs)
{
	ecs->deleteEntitiesOfComponents<CFilmTile>();
	ecs->clearComponents<CFilmTile>();
	ecs->deleteEntitiesOfComponents<CCameraRasterPoint>();
	ecs->clearComponents<CCameraRasterPoint>();
	ecs->clearComponents<CSamples1D>();
	ecs->clearComponents<CSamples2D>();
}


void JobOutputFilm::update(WECS* ecs, HEntity hEntity, CFilm& film) 
{
	uint32_t tileSize = 32;

	std::vector<uint8_t> data;
	data.resize(film.resolution.x()*film.resolution.y() * 3);
	uint32_t tileRowSize = tileSize * 3;
	uint32_t tileCapacity = tileSize * tileSize * 3;
	uint32_t tileRowSizeBytes = tileRowSize * 1;
	uint32_t tileSizeBytes = tileSize * 1;

	uint8_t* mergeArr = data.data();
	uint32_t nAbsTiles = film.resolution.x() / tileSize * film.resolution.y() / tileSize;
	DPoint2i nTiles = film.resolution / tileSize;
	for (uint32_t i = 0; i < nAbsTiles; i++)
	{
		DPoint2i iP = DPoint2i(i % nTiles.x(), i / nTiles.x());



		uint8_t* curTile = film.rgbOutput.data() + i * tileCapacity;

		//std::vector<unsigned char> dataT(curTile, curTile + tileCapacity);
		//std::vector<unsigned char> pngT;
		//unsigned error = lodepng::encode(pngT, dataT, tileSize, tileSize, LCT_RGB, 8);
		//assert(error == 0);

		//
		//std::string istr(1, '0'+i);
		//std::string fn = "test" + istr + ".png";
		//lodepng::save_file(pngT, fn);
		for (uint32_t k = 0; k < tileSize; k++)
		{

			uint8_t* dest = mergeArr + (film.resolution.x()*(tileSize*iP.y() + k) + iP.x()*tileSize) * 3;
			std::memcpy(dest, curTile, tileRowSizeBytes);
			curTile += tileRowSize;
		}
	}

	std::vector<unsigned char> png;
	unsigned error = lodepng::encode(png, data, film.resolution.x(), film.resolution.y(), LCT_RGB, 8);
	assert(error == 0);
	lodepng::save_file(png, "testpng.png");

	//FILE *imageFile;
	//fopen_s(&imageFile, film.outFile.data(), "wb");
	//if (imageFile == NULL)
	//{
	//	perror("ERROR: Cannot open output file");
	//	exit(EXIT_FAILURE);
	//}

	//fprintf(imageFile, "P6\n");               // P6 filetype
	//fprintf(imageFile, "%d %d\n", film.resolution.x(), film.resolution.y());   // dimensions
	//fprintf(imageFile, "255\n");              // Max pixel

	//fwrite(mergeArr, 1, data.size(), imageFile);
	//fclose(imageFile);
}


WPBR_END