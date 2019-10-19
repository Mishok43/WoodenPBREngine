#include "pch.h"
#include "CLightInfiniteArea.h"
#include "MEngine.h"
#include "SLBVH.h"

WPBR_BEGIN

DECL_OUT_COMP_DATA(CPosition)
DECL_OUT_COMP_DATA(CLMapDistribution)


HEntity SLightInfiniteArea::create(Spectrum lemit, const std::string& lightMap)
{
	MEngine& ecs = MEngine::getInstance();
	HEntity hEntity = ecs.createEntity();


	CTextureBindingRGB lMap;
	lMap.filename = lightMap;

	CLight l;
	l.LEmit = lemit;
	ecs.addComponent<CLight>(hEntity, l);
	ecs.addComponent<CSphere>(hEntity);
	ecs.addComponent<CPosition>(hEntity);
	ecs.addComponent<CTextureBindingRGB>(hEntity, std::move(lMap));
	ecs.addComponent<CLightLiComputeRequests>(hEntity);
	ecs.addComponent<CLightLiSampleRequests>(hEntity);
	ecs.addComponent<CLightLeComputeRequests>(hEntity);

	return hEntity;
}


void JobLightInfiniteAreaPreprocessLocation::update(WECS* ecs, HEntity hEntity, CPosition& pos, CSphere& sphere)
{
	const CSceneTreeLBVH& sceneTree = ecs->getComponentsRawData<CSceneTreeLBVH>()[0];
	const DBounds3f& sceneBounds = sceneTree.getSceneBounds();
	sceneBounds.boundingSphere(pos.p, sphere.radius);
}

void JobLightInfiniteAreaPreprocessSampling::update(WECS* ecs, HEntity hEntity, CLight& light, CTextureBindingRGB& lMap)
{
	assert(lMap.isLoaded);

	//wal::AllocatorLinear& tmpALlocator = getAllocatorTemp();
	//float* data = (float*)tmpALlocator.allocMem(lMap.resX*lMap.resY * sizeof(float), sizeof(float));


	std::vector<float> data;
	data.resize(lMap.resX*lMap.resY);
	for (uint32_t y = 0; y < lMap.resY; y++)
	{
		float yN = (float)y / (float)lMap.resY;
		float sinTheta = std::sin(PI*float(y + 0.5f) / float(lMap.resY));
		for (uint32_t x = 0; x < lMap.resX; x++)
		{
			float xN = (float)x / (float)lMap.resX;
			data[x + y * lMap.resY] = STextureSamplerIsotropic::evaluate(DPoint2f(xN, yN), lMap.getTex(ecs)).y();
			data[x + y * lMap.resY] *= sinTheta;
		}
	}
	

	CLMapDistribution lMapDistr;
	lMapDistr.d = DDistributionbPieceWise2D<float>(data.data(), lMap.resX, lMap.resY);
	ecs->addComponent<CLMapDistribution>(hEntity, lMapDistr);
}

void JobLightInfiniteAreaLeCompute::update(WECS* ecs, HEntity hEntity, CLight& l, CLightLeComputeRequests& requests, CTextureBindingRGB& lMap)
{
	for (uint32_t i = 0; i < requests.data.size(); i++)
	{
		HEntity hRequest = requests.data[i];
		const CRayDifferential& ray = ecs->getComponent<CRayDifferential>(hRequest);

		CTextureMappedPoint st;
		st.p = DPoint2f(sphericalPhi(ray.dir) / (2 * PI),
						sphericalTheta(ray.dir) / PI);

		CLightLE le;
		le.L = STextureSamplerIsotropic::evaluate(st, lMap.getTex(ecs));
		ecs->addComponent<CLightLE>(hRequest, std::move(le));
	}

	requests.data.clear();
}


void JobLightInfiniteAreaLiSample::update(WECS* ecs, HEntity hEntity, CLight& l, CLightLiSampleRequests& requests, 
										  CLMapDistribution& distr, CTextureBindingRGB& lMap)
{
	for (uint32_t i = 0; i < requests.data.size(); i++)
	{
		HEntity hRequest = requests.data[i];

		CSamples2D& samples = ecs->getComponent<CSamples2D>(hRequest);
		const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(hRequest);


		CSampledWI wi;
		CSampledLightLI li;
		CSampledLightPDF pdf;
		

		float mapPDF;
		DPoint2f uv = distr.d.sampleContinuous(samples.next(), mapPDF);
		if (mapPDF != 0.0f)
		{
			float theta = uv[1] * PI;
			float phi = uv[0] * 2 * PI;
			float cosTheta = std::cos(theta);
			float sinTheta = std::sin(theta);
			wi = sphericalToCasterian(sinTheta, cosTheta, phi);

			pdf.p = (sinTheta != 0.0f) ? mapPDF / (2 * PI*PI*sinTheta) : 0.0f;
		}
		
		CLightLE le;
		le.L = Spectrum(Spectrum(STextureSamplerIsotropic::evaluate(uv, lMap.getTex(ecs)), SpectrumType::Illuminant)*l.LEmit);


		ecs->addComponent<CSampledWI>(hRequest, std::move(wi));
		ecs->addComponent<CSampledLightLI>(hRequest, std::move(li));
		ecs->addComponent<CSampledLightPDF>(hRequest, std::move(pdf));
	}

	requests.data.clear();
}

void JobLightInfiniteAreaLiCompute::update(WECS* ecs, HEntity hEntity, CLight& l, CLightLiComputeRequests& requests,
										  CLMapDistribution& distr, CTextureBindingRGB& lMap)
{
	for (uint32_t i = 0; i < requests.data.size(); i++)
	{
		HEntity hRequest = requests.data[i];

		CSampledWI& wi = ecs->getComponent<CSampledWI>(hRequest);
		const CSurfaceInteraction& si = ecs->getComponent<CSurfaceInteraction>(hRequest);

		CSampledLightLI li;
		CSampledLightPDF pdf;

		float theta = sphericalTheta(wi);
		float phi = sphericalPhi(wi);
		DPoint2f uv = DPoint2f(phi / (2 * PI), theta / PI);
		float sinTheta = std::sin(theta);
		if (sinTheta != 0.0f)
		{
			 pdf.p =  distr.d.pdf(DPoint2f(phi / (2 * PI), theta / (PI))) / (2 * PI*PI*sinTheta);
			 li = Spectrum(Spectrum(STextureSamplerIsotropic::evaluate(uv, lMap.getTex(ecs)), SpectrumType::Illuminant)*l.LEmit);
		}

		ecs->addComponent<CSampledLightLI>(hRequest, std::move(li));
		ecs->addComponent<CSampledLightPDF>(hRequest, std::move(pdf));
	}

	requests.data.clear();
}

WPBR_END