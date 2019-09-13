#pragma once

#include "pch.h"
#include <map>

#include <algorithm>
#include "CTextureMapping.h"
#include "WoodenAllocators/Allocator.h"
#include "CSufraceInteraction.h"
#include "WoodenECS/Job.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "RGBSpectrum.h"

WPBR_BEGIN

template<typename T>
class TextureConst
{
	T sample(const CSurfaceInteraction& si) const
	{
		return value;
	}

	T value;
};


struct CTextureInfo
{
	std::string filename;
	HEntity handle;

	DECL_MANAGED_DENSE_COMP_DATA(CTextureInfo, 16)
}; DECL_OUT_COMP_DATA(CTextureInfo)

struct CTextureRGB
{
	std::string filename;
	std::vector<std::vector<RGBSpectrum, AllocatorAligned<RGBSpectrum>>> mips;
	std::vector<DPoint2i, AllocatorAligned<DPoint2i>> resolutions;
	std::vector<DPoint2i, DPoint2i> mipNBlocks;

	static constexpr uint8_t blockSize = 4;
	uint32_t getTexelIndex(const DPoint2i& p, uint8_t mip)
	{
		const DPoint2i& nBlocks = mipNBlocks[mip];
		constexpr uint8_t blockArea = blockSize*blockSize;
		DPoint2i block = p / blockSize;

		uint32_t iBlock = block.x() + block.y()*nBlocks.x();
		uint32_t i = iBlock * blockArea+(p.x()%blockSize)+(p.y()%blockSize)*blockSize;
		return i;
	}

	RGBSpectrum& getTexel(const DPoint2i& p, uint8_t mip)
	{
		return mips[mip][getTexelIndex(p, mip)];
	}

	RGBSpectrum& getTexel(uint32_t x, uint32_t y, uint8_t mip)
	{
		return mips[mip][getTexelIndex(DPoint2i(x, y), mip)];
	}

	DECL_MANAGED_DENSE_COMP_DATA(CTextureRGB, 16)
}; DECL_OUT_COMP_DATA(CTextureRGB)

struct CTextureRMIP
{
	std::vector<float, AllocatorAligned<float>> texels;

	DECL_MANAGED_DENSE_COMP_DATA(CTextureRMIP, 16)
}; DECL_OUT_COMP_DATA(CTextureRMIP)

struct CTextureMIPHandle : public HEntity
{
	CTextureMIPHandle() = default;
	CTextureMIPHandle(HEntity h): HEntity(h){ }

	DECL_MANAGED_DENSE_COMP_DATA(CTextureMIPHandle, 16)
}; DECL_OUT_COMP_DATA(CTextureMIPHandle)


struct CSurfaceInteractionHandle : public HEntity
{
	CSurfaceInteractionHandle() = default;
	CSurfaceInteractionHandle(HEntity h) : HEntity(h)
	{}

	DECL_MANAGED_DENSE_COMP_DATA(CSurfaceInteractionHandle, 16)
}; DECL_OUT_COMP_DATA(CSurfaceInteractionHandle)

class JobSampleTextureRGB : public Job
{
	void sample(const CSurfaceInteraction& surfInter,
		   const CTextureMappedPoint& st,
		   const CTextureInfo& tInfo,
		   const CTextureRGB& tMIP) 
	{

	}
};

class JobLoadTexture: public Job
{
public:
	static std::map<std::string, HEntity> textures;



	virtual void update(WECS* ecs) override
	{
		ComponentsGroup<CTextureInfo> texInfos = queryComponentsGroup<CTextureInfo>();
		for_each([&](HEntity hEntity, CTextureInfo& textureInfo)
		{
			std::map<std::string, HEntity>::const_iterator it = textures.find(textureInfo.filename);
			if (it != textures.cend())
			{
				ecs->addComponent<CTextureMIPHandle>(hEntity, std::move(it->second));
				return;
			}


			DPoint2i resolutions;
			std::vector<RGBSpectrum, AllocatorAligned<RGBSpectrum>> texels = readImagePNG(textureInfo.filename, resolutions);

			assert(!texels.empty());

			for (uint32_t i = 0; i < texels.size(); i++)
			{
				for (uint8_t j = 0; j < 3; j++)
				{
					texels[i][j] = gammaCorrectInv(texels[i][j]);
				}
			}

			HEntity hTexture = ecs->createEntity();
			ecs->addComponent<CTextureRGB>(hTexture, std::move(texels));

			textureInfo.handle = hTexture;
			textures[textureInfo.filename] = hTexture;
			
		}, texInfos);
	}

	CTextureRGB generateMIPs(std::vector<RGBSpectrum, AllocatorAligned<RGBSpectrum>> texels, DPoint2i resolution)
	{
		uint16_t nLevels = 1 + log2(std::max(resolution[0], resolution[1]));
		CTextureRGB tex;
		tex.resolutions.resize(nLevels);
		tex.mips.resize(nLevels);
		tex.mipNBlocks.resize(nLevels);

		tex.resolutions[0] = std::move(resolution);
		tex.mipNBlocks[0] = tex.resolutions[0] / CTextureRGB::blockSize;
		for (uint32_t y = 0; y < tex.resolutions[0].y(); y++)
		{
			for (uint32_t x = 0; x < tex.resolutions[0].x(); x++)
			{
				tex.getTexel(x, y, 0) = std::move(texels[x + y * tex.resolutions[0].x()]);
			}
		}

		tex.mips[0] = std::move(texels);
		tex.resolutions[0] = std::move(resolution);

		for (uint16_t i = 1; i < nLevels; i++)
		{
			DPoint2i res = maxv(DPoint2i(1, 1), tex.resolutions[i - 1] / 2);
			tex.mips[i].resize(res.x()*res.y());
			tex.resolutions[i] = res;
			tex.mipNBlocks[i] = res / CTextureRGB::blockSize;

			std::vector<RGBSpectrum, AllocatorAligned<RGBSpectrum>>& mip = tex.mips[i];	
			for (uint32_t y = 0; y < res.y()-1; y++)
			{
				for (uint32_t x = 0; x < res.x()-1; x++)
				{
					tex.getTexel(x, y, i) = (tex.getTexel(2 * x, 2 * y) + tex.getTexel(2 * x + 1, 2 * y) +
											 tex.getTexel(2 * x, 2 * y + 1) + tex.getTexel(2 * x + 1, 2 * y + 1))*0.25;
				}

				tex.getTexel(res.x()-1, y, i) = (tex.getTexel(tex.resolutions[i-1].x()-1, 2 * y) +
										 tex.getTexel(tex.resolutions[i-1].x()-1, 2 * y + 1))*0.5;
			}

			for (uint32_t x = 0; x < res.x() - 1; x++)
			{
				tex.getTexel(x, res.y()-1, i) = (tex.getTexel(2 * x, tex.resolutions[i - 1].y()-1) + 
												 tex.getTexel(2 * x + 1, tex.resolutions[i - 1].y() - 1))*0.5;
			}

			tex.getTexel(res.x() - 1, res.y() - 1, i) = tex.getTexel(tex.resolutions[i - 1].x() - 1, tex.resolutions[i - 1].y() - 1);
		}
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

	inline float gammaCorrectInv(float value)
	{
		if (value <= 0.04045f)
			return value * 1.f / 12.92f;
		return std::pow((value + 0.055f) * 1.f / 1.055f, 2.4f);
	}


	static std::vector<RGBSpectrum, AllocatorAligned<RGBSpectrum>> readImagePNG(const std::string &name, DPoint2i& resolution)
	{
		unsigned char *rgb;
		unsigned w, h;
		unsigned int error = lodepng_decode24_file(&rgb, &w, &h, name.c_str());
		assert(error == 0);
		resolution.x = w;
		resolution.y = h;

		std::vector<RGBSpectrum, AllocatorAligned<RGBSpectrum>> ret(resolution.x*resolution.y);
		unsigned char *src = rgb;
		for (unsigned int y = 0; y < h; ++y)
		{
			for (unsigned int x = 0; x < w; ++x, src += 3)
			{
				ret[y * resolution.x + x] = DVector3f(src[0], src[1], src[2]) / 255.0f;
			}
		}

		free(rgb);
		return ret;
	}
};


WPBR_END

