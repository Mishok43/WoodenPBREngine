#include "pch.h"
#include "CTexture.h"

WPBR_BEGIN
DECL_OUT_COMP_DATA(CTextureR)
DECL_OUT_COMP_DATA(CTextureRGB)
DECL_OUT_COMP_DATA(CTextureBindingRGB)
DECL_OUT_COMP_DATA(CTextureBindingR)
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
	ComponentsGroup<CTextureBindingRGB> texInfos = queryComponentsGroup<CTextureBindingRGB>();
	for_each([&](HEntity hEntity, CTextureBindingRGB& textureInfo)
	{
		std::map<std::string, HEntity>::const_iterator it = textures.find(textureInfo.filename);
		if (it != textures.cend())
		{
			textureInfo.tex = it->second;
			return;
		}


		DPoint2i resolutions;
		std::vector<RGBSpectrum, AllocatorAligned2<RGBSpectrum>> texels = readImagePNG(textureInfo.filename, resolutions);

		assert(!texels.empty());

		//for (uint32_t i = 0; i < texels.size(); i++)
		//{
		//	for (uint8_t j = 0; j < 3; j++)
		//	{
		//		texels[i][j] = gammaCorrectInv(texels[i][j]);
		//	}
		//}

		HEntity hTexture = ecs->createEntity();
		CTextureRGB texRGB = generateMIPs(std::move(texels), resolutions);
		ecs->addComponent<CTextureRGB>(hTexture, std::move(texRGB));

		textureInfo.tex = hTexture;
		textures[textureInfo.filename] = hTexture;
	}, texInfos);
}

CTextureRGB JobLoadTextureRGB::generateMIPs(std::vector<RGBSpectrum, AllocatorAligned2<RGBSpectrum>> texels, DPoint2i resolution)
{
	uint16_t nLevels = log2(max(resolution[0], resolution[1]))-1;
	CTextureRGB tex;
	tex.resolutions.resize(nLevels);
	tex.mips.resize(nLevels);
	tex.mipNBlocks.resize(nLevels);

	tex.resolutions[0] = std::move(resolution);
	tex.mipNBlocks[0] = tex.resolutions[0] / CTextureRGB::blockSize;
	tex.mips[0].resize(tex.resolutions[0].x()*tex.resolutions[0].y());
	for (uint32_t y = 0; y < tex.resolutions[0].y(); y++)
	{
		for (uint32_t x = 0; x < tex.resolutions[0].x(); x++)
		{
			tex.texel(x, y, 0) = std::move(texels[x + y * tex.resolutions[0].x()]);
		}
	}


	for (uint16_t i = 1; i < nLevels; i++)
	{
		DPoint2i res = maxv(DPoint2i(1, 1), tex.resolutions[i - 1] / 2);
		tex.mips[i].resize(res.x()*res.y());
		tex.resolutions[i] = res;
		tex.mipNBlocks[i] = res / CTextureRGB::blockSize;

		std::vector<RGBSpectrum, AllocatorAligned2<RGBSpectrum>>& mip = tex.mips[i];
		for (uint32_t y = 0; y < res.y() - 1; y++)
		{
			for (uint32_t x = 0; x < res.x() - 1; x++)
			{
				tex.texel(x, y, i) = (tex.texel(2 * x, 2 * y, i-1) + tex.texel(2 * x + 1, 2 * y, i-1) +
									  tex.texel(2 * x, 2 * y + 1, i-1) + tex.texel(2 * x + 1, 2 * y + 1, i-1))*0.25;
			}

			tex.texel(res.x() - 1, y, i) = (tex.texel(tex.resolutions[i - 1].x() - 1, 2 * y, i-1) +
											tex.texel(tex.resolutions[i - 1].x() - 1, 2 * y + 1, i-1))*0.5;
		}

		for (uint32_t x = 0; x < res.x() - 1; x++)
		{
			tex.texel(x, res.y() - 1, i) = (tex.texel(2 * x, tex.resolutions[i - 1].y() - 1, i-1) +
											tex.texel(2 * x + 1, tex.resolutions[i - 1].y() - 1, i-1))*0.5;
		}

		tex.texel(res.x() - 1, res.y() - 1, i) = tex.texel(tex.resolutions[i - 1].x() - 1, tex.resolutions[i - 1].y() - 1, i-1);
	}
	return tex;
}

std::vector<RGBSpectrum, AllocatorAligned2<RGBSpectrum>> JobLoadTextureRGB::readImagePNG(const std::string &name, DPoint2i& resolution)
{
	unsigned char *rgb;
	unsigned w, h;
	unsigned int error = lodepng_decode24_file(&rgb, &w, &h, name.c_str());
	assert(error == 0);
	resolution.x() = w;
	resolution.y() = h;

	std::vector<RGBSpectrum, AllocatorAligned2<RGBSpectrum>> ret(resolution.x()*resolution.y());
	unsigned char *src = rgb;
	for (unsigned int y = 0; y < h; ++y)
	{
		for (unsigned int x = 0; x < w; ++x, src += 3)
		{
			ret[y * resolution.x() + x] = DVector3f(src[0], src[1], src[2]) / 255.0f;
		}
	}

	free(rgb);
	return ret;
}


WPBR_END

