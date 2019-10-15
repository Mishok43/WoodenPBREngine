#include "pch.h"
#include "CTexture.h"

WPBR_BEGIN
DECL_OUT_COMP_DATA(CTexture2DR)
DECL_OUT_COMP_DATA(CTexture2DRGB)
DECL_OUT_COMP_DATA(CTextureBinding2DRGB)
DECL_OUT_COMP_DATA(CTextureBinding2DR)
DECL_OUT_COMP_DATA(CSurfaceInteractionHandle)
DECL_OUT_COMP_DATA(CTextureSamplerIsotropic)
DECL_OUT_COMP_DATA(CTextureSamplerAnistropic)
DECL_OUT_COMP_DATA(CFilterTableGaussing)

std::map<std::string, HEntity> JobLoadTextureRGB::textures;


void JobFilterTableGaussing::update(WECS* ecs, uint8_t iThread) 
{
	uint32_t n = queryComponentsGroup<CFilterTableGaussing>().size();
	uint32_t sliceSize = (n + getNumThreads() - 1) / getNumThreads();
	ComponentsGroupSlice comps = queryComponentsGroupSlice<CFilterTableGaussing>(Slice(iThread*sliceSize, sliceSize));
	for_each([](HEntity hEntity, CFilterTableGaussing& table)
	{
		table.weights.resize(table.size);
		for (int i = 0; i < table.size; ++i)
		{
			float alpha = 2;
			float r2 = float(i) / float(table.size - 1);
			table.weights[i] = std::exp(-alpha * r2) - std::exp(-alpha);
		}

	}, comps);
}


void JobLoadTextureRGB::update(WECS* ecs)
{
	ComponentsGroup<CTextureBinding2DRGB> texInfos = queryComponentsGroup<CTextureBinding2DRGB>();
	for_each([&](HEntity hEntity, CTextureBinding2DRGB& textureInfo)
	{
		SLoadTexture::load<DVectorPacked3f, CTexture2DRGB>(textures, textureInfo);

	}, texInfos);
}

void JobLoadTextureR::update(WECS* ecs)
{
	ComponentsGroup<CTextureBinding2DR> texInfos = queryComponentsGroup<CTextureBinding2DR>();
	for_each([&](HEntity hEntity, CTextureBinding2DR& textureInfo)
	{
		SLoadTexture::load<DVectorPacked1f, CTexture2DR>(textures, textureInfo);

	}, texInfos);
}


WPBR_END

