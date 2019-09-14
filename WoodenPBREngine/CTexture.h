#pragma once

#include "pch.h"
#include <map>

#include <algorithm>
#include "CTextureMapping.h"
#include "WoodenAllocators/Allocator.h"
#include "CSufraceInteraction.h"
#include "WoodenECS/Job.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/Utils.h"
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
	uint32_t iTexel(const DPoint2i& p, uint8_t mip) const
	{
		const DPoint2i& nBlocks = mipNBlocks[mip];
		constexpr uint8_t blockArea = blockSize*blockSize;
		DPoint2i block = p / blockSize;

		uint32_t iBlock = block.x() + block.y()*nBlocks.x();
		uint32_t i = iBlock * blockArea+(p.x()%blockSize)+(p.y()%blockSize)*blockSize;
		return i;
	}

	RGBSpectrum& texel(const DPoint2i& p, uint8_t mip) const
	{
		return mips[mip][iTexel(p, mip)];
	}

	RGBSpectrum& texel(uint32_t x, uint32_t y, uint8_t mip) const
	{
		return mips[mip][iTexel(DPoint2i(x, y), mip)];
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


struct CTextureRGBSamplerIsotropic
{	 
	DECL_MANAGED_DENSE_COMP_DATA(CTextureRGBSamplerIsotropic, 1)
}; DECL_OUT_COMP_DATA(CTextureRGBSamplerIsotropic)

struct CTextureRGBSamplerAnistropic
{
	float maxAnisotropy;
	HEntity filterTable;
	DECL_MANAGED_DENSE_COMP_DATA(CTextureRGBSamplerAnistropic, 1)
}; DECL_OUT_COMP_DATA(CTextureRGBSamplerAnistropic)

struct CFilterTableGaussing
{
	std::vector<float, AllocatorAligned<float>> weights;
	uint32_t size;
};

class JobFilterTableGaussing: public JobParallazible
{
	void updateNStartThreads(uint8_t nWorkThreads) override
	{
		nThreads = std::min(queryComponentsGroup<CFilterTableGaussing>().size(), nWorkThreads);
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t n = queryComponentsGroup<CFilterTableGaussing>().size();
		uint32_t sliceSize = (n + nThreads - 1) / nThreads;
		ComponentsGroupSlice comps = queryComponentsGroupSlice<CFilterTableGaussing>(Slice(iThread*sliceSize, sliceSize));
		for_each([](HEntity hEntity, CFilterTableGaussing& table)
		{
			for (int i = 0; i < table.size; ++i)
			{
				float alpha = 2;
				float r2 = float(i) / float(table.size - 1);
				table.weights[i] = std::exp(-alpha * r2) - std::exp(-alpha);
			}
		
		}, comps);
	}
};

class JobSampleAnisotropicTextureRGB : public Job
{



	RGBSpectrum sample(
		   CTextureMappedPoint& st,
		   const CTextureRGBSamplerAnistropic& sampler,
		   const CFilterTableGaussing& filterTable,
		   const CTextureRGB& tex) 
	{
		DVector2f& dst0 = st.dstdx;
		DVector2f& dst1 = st.dstdy;
		const float& maxAnisotropy = sampler.maxAnisotropy;

		if (st.dstdx.length2() < st.dstdx.length2())
			std::swap(dst0, dst1);
		float majorLength = dst0.length();
		float minorLength = dst1.length();
		
		if (minorLength * maxAnisotropy < majorLength && minorLength > 0)
		{
			float scale = majorLength / (minorLength * maxAnisotropy);
			dst1 *= scale;
			minorLength *= scale;
		}

		if (minorLength == 0)
		{
			return triangle(st.p, tex, 0);
		}

		
		uint32_t nLevels = tex.mips.size();
		float lod = std::max(0.0f, nLevels - 1.0f + log2(minorLength));
		int ilod = std::floor(lod);
		return wml::lerp(ewa(st, tex, ilod),
					ewa(st, tex, ilod+1), lod - ilod);
	}

	RGBSpectrum triangle(const DPoint2f& st,
						 const CTextureRGB& tex,
						 const CFilterTableGaussing& filter,
						 uint32_t mip)
	{
		uint32_t nLevels = tex.mips.size();
		mip = wml::clamp(mip, 0, nLevels - 1);
		float s = st[0] * tex.resolutions[mip].x() - 0.5f;
		float t = st[1] * tex.resolutions[mip].y() - 0.5f;
		int s0 = std::floor(s), t0 = std::floor(t);
		float ds = s - s0, dt = t - t0;
		return	tex.texel(s0, t0, mip)*(1 - ds) * (1 - dt) +
				tex.texel(s0, t0 + 1, mip)*(1 - ds) * dt +
				tex.texel(s0 + 1, t0, mip)*ds * (1 - dt) +
				tex.texel(s0 + 1, t0 + 1, mip)*ds * dt;
	}

	RGBSpectrum ewa(CTextureMappedPoint& p,
					const CTextureRGB& tex,
					uint32_t mip)
	{
		uint32_t nLevels = tex.mips.size();

		DVector2f& st = p.p;
		DVector2f& dst0 = p.dstdx;
		DVector2f& dst1 = p.dstdy;


		st = tex.resolutions[mip]*st   - 0.5f;
		dst0 *= tex.resolutions[mip];
		dst1 *= tex.resolutions[mip];
	
		
		DVector3f abc;
		abc[0] = dst0[1] * dst0[1] + dst1[1] * dst1[1] + 1;
		abc[1] = -2 * (dst0[0] * dst0[1] + dst1[0] * dst1[1]);
		abc[2] = dst0[0] * dst0[0] + dst1[0] * dst1[0] + 1;

		const float& A = abc[0];
		const float& B = abc[1];
		const float& C = abc[2];

		float invF = 1 / (A * C - B * B * 0.25f);
		abc *= invF;

		float det = -B * B + 4 * A * C;
		float invDet = 1 / det;
		float uSqrt = std::sqrt(det * C),vSqrt = std::sqrt(A * det);

		DVector2f uvSqrt(C, A);
		uvSqrt *= det;
		uvSqrt = sqrt(uvSqrt);
		uvSqrt *= 2 * invDet;

		DVector2f st0 = st - uvSqrt;
		DVector2f st1 = st + uvSqrt;

		int s0 = std::ceil(st0[0]);
		int t0 = std::ceil(st0[1]);

		int s1 = std::floor(st1[0]);
		int t1 = std::floor(st1[1]);
		
		RGBSpectrum sum(0.f);
		float sumWts = 0;
		for (int it = t0; it <= t1; ++it)
		{
			float tt = it - st[1];
			for (int is = s0; is <= s1; ++is)
			{
				float ss = is - st[0];
				
				float r2 = A * ss * ss + B * ss * tt + C * tt * tt;
				if (r2 < 1)
				{
					int index = std::min((int)(r2 * filter.size),
										 filter.size- 1);
					float weight = filter.weights[index];
					sum += tex.texel(is, it, mip) * weight;
					sumWts += weight;
				}

			}
		}
		return sum / sumWts;
	}
};


class JobSampleIsotropicTextureRGB : public Job
{
	RGBSpectrum sample(
		   const CTextureMappedPoint& st,
		   const CTextureRGBSamplerIsotropic& sampler,
		   const CTextureRGB& tex) 
	{
		float width = 2 * std::max(std::max(st.dstdx[0], st.dstdx[1]),
								   std::max(st.dstdy[0], st.dstdy[1]));

		uint32_t nLevels = tex.mips.size();
		float level = nLevels - 1 + log2(std::max(width, 0));
		if (level < 0)
		{
			return triangle(st.p, tex, 0);
		}
		else if (level >= nLevels - 1)
		{
			return tex.texel(0, 0, nLevels - 1);
		}
		else
		{
			int iLevel = std::floor(level);
			float delta = level - iLevel;
			return lerp(triangle(st.p, tex, iLevel), triangle(st.p, tex, iLevel + 1), delta);
		}
	}

	RGBSpectrum triangle(const DPoint2f& st,
						 const CTextureRGB& tex,
						 uint32_t mip)
	{
		uint32_t nLevels = tex.mips.size();
		mip = wml::clamp(mip, 0, nLevels - 1);
		float s = st[0] * tex.resolutions[mip].x() - 0.5f;
		float t = st[1] * tex.resolutions[mip].y() - 0.5f;
		int s0 = std::floor(s), t0 = std::floor(t);
		float ds = s - s0, dt = t - t0;
		return	tex.texel(s0, t0, mip)*(1 - ds) * (1 - dt) +
				tex.texel(s0, t0 + 1, mip)*(1 - ds) * dt +
				tex.texel(s0 + 1, t0, mip)*ds * (1 - dt) +
				tex.texel(s0 + 1, t0 + 1, mip)*ds * dt;
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
				tex.texel(x, y, 0) = std::move(texels[x + y * tex.resolutions[0].x()]);
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
					tex.texel(x, y, i) = (tex.texel(2 * x, 2 * y) + tex.texel(2 * x + 1, 2 * y) +
											 tex.texel(2 * x, 2 * y + 1) + tex.texel(2 * x + 1, 2 * y + 1))*0.25;
				}

				tex.texel(res.x()-1, y, i) = (tex.texel(tex.resolutions[i-1].x()-1, 2 * y) +
										 tex.texel(tex.resolutions[i-1].x()-1, 2 * y + 1))*0.5;
			}

			for (uint32_t x = 0; x < res.x() - 1; x++)
			{
				tex.texel(x, res.y()-1, i) = (tex.texel(2 * x, tex.resolutions[i - 1].y()-1) + 
												 tex.texel(2 * x + 1, tex.resolutions[i - 1].y() - 1))*0.5;
			}

			tex.texel(res.x() - 1, res.y() - 1, i) = tex.texel(tex.resolutions[i - 1].x() - 1, tex.resolutions[i - 1].y() - 1);
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

