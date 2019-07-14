#include "pch.h"
#include "SIntergratorWhitted.h"
#include "SLighting.h"
#include "SCollision.h"

WPBR_BEGIN

template<typename AllocT>
CSpectrum SIntegratorWhitted<AllocT>::lightIn(const CRayDifferntial& ray, CSampler& sampler, AllocT& alloc, uint32_t depth)
{
	CSufraceInteraction surfIntersct = SCollision::raycast(ray, cScene->primitive);
	CSpectrum lightIn(.0f);

	if (surfIntersct.bIntersect)
	{
		wal::DVector3f surfNormal = surfIntersct.surfNormal;
		wal::DVector3f surfToRay = surfIntersct.surfToRay;

		lightIn += SLighting::lightEmitted(surfIntersct, surfToRay);
		SLighting::computeScatteringFunctions(surfIntersct, alloc);

		engine->for_each<CLight>([&ray, &surfToRay, &surfIntersct, &LightIn](CLight& light)
		{
			wal::DVector3f surfToLight;
			float pdf;
			VisibilityTester visibility;

			CSpectrum lightInFromLight = SLighting::lightSample(light, surfIntersct, sampler,
												  surfToLight, pdf, visibility);
			if (lightInFromLight.isBlack() || pdf == 0.0)
			{
				return;
			}

			
			CSpectrum f = SLighting::lightBSDF(surfIntersct, surfToRay, surfToLight);
			if (!if.isBlock() && visbility.Unoccluded(scene))
			{
				LightIn += f * lightInFromLight* wal::DVector3f::absDot(surfNormal, surfToLight) / pdf;
			}

			lightIn += 
		});

		if (++depth < maxDepth)
		{
			lightIn(....);
		}
	}
	else
	{
		engine->for_each<CLight>([&ray](CLight& light)
		{
			lightIn += SLighting::lightEmitted(light, ray);
		});
	}
}

template<typename AllocT>
CSpectrum SIntegratorWhitted<AllocT>::specularReflect(
	const CRayDifferential& ray,
	const CSurfaceInteraction& surfIntersct,
	CSampler& sampler,
	AllocT& alloc,
	uint32_t depth) const
{
	DVector3f surfToRay = surfIntersct.surfToRay;
	DVector3f reflectedRay;

	float pdf;
	BxDFType type = BxDFType(BSDF_REFLECTION | BSDF_SPECULAR);
	CSpectrum f = SLighting::funcSample(surfToRay, surfIntersct, sampler, pdf, reflectedRay, type);

	float dotReflNorm = DVector3f::absDot(reflectedRay, surfIntersct.surfNormal);
	if (pdf > 0 && !f.isBlack() && dotReflNorm != 0)
	{
		return f * lightIn(reflectedRay, sampler, alloc, depth + 1);
	}
	else
	{
		return Spectrum(.f);
	}
}


template<typename AllocT>
void SIntegratorWhitted<AllocT>::renderTile(DVector2u&& tileBounds,
				CSampler&& tileSampler,
				CFileTile&& tileFilm)
{

	AllocT allocPerSample(1 << 14); // 16 kilobyte

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
				L = lightIn(ray, cScene, tileSampler, allocPerSample);
			}

			tileFilm.addSample(cameraSample.pFilm, L, rayWeight);

			allocPerSample.reset();
		} while (tileSampler.startNextSample());
	}

	cCamera->pFilm.mergeFilmTile(std::move(tileFilm));
}

template<typename AllocT>
void SIntegratorWhitted<AllocT>::updateECS(wecs::WECS& _engine, CScene& scene,
										   CCamera& camera, CSampler& sampler)
{
	preprocess(scene, sampler);

	DBounds2u sampleBounds = camera.film.getSampleBounds();
	DVector2u sampleExtent = sampleBounds.diagonal();
	const uint8_t tileSize = 16;
	DVector2u nTiles((sampleExtent.x() + tileSize - 1) / tileSize,
		(sampleExtent.y() + tileSize - 1) / tileSize);

	cScene = &scene;
	cCamera = &camera;
	engine = &_engine;

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

WPBR_END


