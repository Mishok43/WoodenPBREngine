#pragma once

#include "pch.h"
#include "CSufraceInteraction.h"
#include "CPrimitive.h"
#include "CScene.h"
#include "CCamera.h"
#include "CSampler.h"
#include "WoodenAllocators/AllocatorLinear.h"
#include "WoodenMathLibrarry/DVector.h"

WPBR_BEGIN
class SIntegratorSampler
{
	using cmp_type_list = typename wecs::type_list<CScene, CCamera, CSampler>;

	SIntegratorSampler()
	{

	}

	void preprocess(CScene& scene, CSampler& sampler)
	{

	}

	template<typename AllocT>
	CSpectrum Li(const CRayDifferntial& ray, CScene* scene,
				CSampler& sampler, AllocT& alloc, uint32_t depth = 0)
	{
		
	}

	void renderTile(DVector2u&& tileBounds,
					CSampler&& tileSampler,
					CFileTile&& tileFilm)
	{

		wal::AllocatorLinear allocPerSample(1 << 14); // 16 kilobyte

		for (DVector2u pixel : tileBounds)
		{
			tileSampler.startPixel(pixel);
			do
			{
				CCameraSample cameraSample = tileSampler.getCameraSample(pixel);
				CRayDifferential ray;
				float rayWeight = cCamera->generateRayDifferential(cameraSample, &ray);
				ray.scaleDifferentials(1.0 / sqrt(tileSampler.samplesPerPixel));

				CSpectrum L(0.0f);
				if (rayWeight > 0)
				{
					L = Li(ray, cScene, tileSampler, allocPerSample);
				}

				tileFilm.addSample(cameraSample.pFilm, L, rayWeight);

				allocPerSample.reset();
			} while (tileSampler.startNextSample());
		}

		cCamera->pFilm.mergeFilmTile(std::move(tileFilm));
	}

	void updateECS(wecs::WECS& ecs, CScene& scene, CCamera& camera, CSampler& sampler)
	{
		preprocess(scene, sampler);

		DBounds2u sampleBounds = camera.film.getSampleBounds();
		DVector2u sampleExtent = sampleBounds.diagonal();
		const uint8_t tileSize = 16;
		DVector2u nTiles((sampleExtent.x() + tileSize - 1) / tileSize,
						(sampleExtent.y() + tileSize - 1) / tileSize);
		
		cScene = &scene;
		cCamera = &camera;
		cECS = &ecs;

		for (size_t i = 0; i < nTiles.x(); ++i)
		{
			for (size_t j = 0; j < nTiles.y(); ++j)
			{
				DVector2i tile = DVector2i(i, j);

				uint32_t seed = tile.x() + tile.y()*nTiles.x();
				CSampler tileSampler = sampler.clone(seed);

				uint32_t boundMinX = sampleBounds.pMin().x() + tile.x*tileSize;
				uint32_t boundMaxX = std::min(boundMinX + tileSize, sampleBounds.pMax().x());
				
				uint32_t boundMinY = sampleBounds.pMin().y() + tile.y*tileSize;
				uint32_t boundMaxY = std::min(boundMinY + tileSize, sampleBounds.pMax().y());

				DBounds2u tileBounds(DVector2u(boundMinX, boundMinY), 
										  DVector2u(boundMaxX, boundMaxY));
				
				CFilmTile tileFilm = camera.film.getFilmTile(tileBounds);

				renderTile(std::move(tileBounds),
						   std::move(tileSampler),
						   std::move(tileFilm));
			}
		}

		camera.film.writeImage();
	}

protected:
	CScene* cScene;
	CCamera* cCamera;
	WECS* cECS;
};

WPBR_END

