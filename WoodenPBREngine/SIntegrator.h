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
class SIntegrator
{
	using cmp_type_list = typename wecs::type_list<CScene>;

	void updateECS(wecs::WECS& ecs, CScene& scene)
	{

	}
};

class SSamplerIntegrator
{
	using cmp_type_list = typename wecs::type_list<CScene, CCamera, CSampler>;

	SSamplerIntegrator()
	{

	}

	void preprocess(CScene& scene, CSampler& sampler)
	{

	}

	void renderTile(wml::DVector2u&& tileBounds,
					CSampler&& tileSampler,
					CFileTile&& tileFilm)
	{

		wal::AllocatorLinear allocPerSample(1 << 14); // 16 kilobyte

		for (wml::DVector2u pixel : tileBounds)
		{
			tileSampler.startPixel(pixel);
			CCameraSample cameraSample = tileSampler.getCameraSample(pixel);
			do
			{
				

				allocPerSample.reset();
			}while(tileSampler.startNextSample())
		}
	}

	void updateECS(wecs::WECS& ecs, CScene& scene, CCamera& camera, CSampler& sampler)
	{
		preprocess(scene, sampler);

		wml::DBounds2u sampleBounds = camera.film.getSampleBounds();
		wml::DVector2u sampleExtent = sampleBounds.diagonal();
		const uint8_t tileSize = 16;
		wml::DVector2u nTiles((sampleExtent.x() + tileSize - 1) / tileSize,
						(sampleExtent.y() + tileSize - 1) / tileSize);
		
		curScene = &scene;
		curCamera = &camera;

		for (size_t i = 0; i < nTiles.x(); ++i)
		{
			for (size_t j = 0; j < nTiles.y(); ++j)
			{
				wml::DVector2i tile = wml::DVector2i(i, j);

				uint32_t seed = tile.x() + tile.y()*nTiles.x();
				CSampler tileSampler = sampler.clone(seed);

				uint32_t boundMinX = sampleBounds.pMin().x() + tile.x*tileSize;
				uint32_t boundMaxX = std::min(boundMinX + tileSize, sampleBounds.pMax().x());
				
				uint32_t boundMinY = sampleBounds.pMin().y() + tile.y*tileSize;
				uint32_t boundMaxY = std::min(boundMinY + tileSize, sampleBounds.pMax().y());

				wml::DBounds2u tileBounds(wml::DVector2u(boundMinX, boundMinY), 
										  wml::DVector2u(boundMaxX, boundMaxY));
				
				CFilmTile tileFilm = camera.film.getFilmTile(tileBounds);

				renderTile(std::move(tileBounds),
						   std::move(tileSampler),
						   std::move(tileFilm));
			}
		}
	}

protected:
	CScene* curScene;
	CCamera* curCamera;
};

WPBR_END

