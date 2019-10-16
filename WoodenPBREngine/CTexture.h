#pragma once

#include "pch.h"
#include <map>
#include "CTexture.h"
#include <algorithm>
#include "lodepng.h"
#include "CTextureMapping.h"
#include "WoodenAllocators/Allocator.h"
#include "CSufraceInteraction.h"
#include "WoodenECS/Job.h"
#include "WoodenMathLibrarry/DPoint.h"
#include "WoodenMathLibrarry/Utils.h"
#include "RGBSpectrum.h"
#include "MEngine.h"


WPBR_BEGIN
//
//
//struct CTextureSpecular : public CTextureBindingR
//{
//	DECL_MANAGED_DENSE_COMP_DATA(CTextureSpecular, 1)
//}; DECL_OUT_COMP_DATA(CTextureSpecular)
//
//struct CTextureDiffuse : public CTextureBindingRGB
//{
//	DECL_MANAGED_DENSE_COMP_DATA(CTextureDiffuse, 1)
//}; DECL_OUT_COMP_DATA(CTextureDiffuse)
//
//
//struct CTextureRoughness : public CTextureBindingR
//{
//	DECL_MANAGED_DENSE_COMP_DATA(CTextureRoughness, 1)
//}; DECL_OUT_COMP_DATA(CTextureRoughness)
//
//
//struct CTextureBumpMap : public CTextureBindingR
//{
//	DECL_MANAGED_DENSE_COMP_DATA(CTextureBumpMap, 1)
//}; DECL_OUT_COMP_DATA(CTextureBumpMap)


struct CSurfaceInteractionHandle : public HEntity
{
	CSurfaceInteractionHandle() = default;
	CSurfaceInteractionHandle(HEntity h) : HEntity(h)
	{}

	DECL_MANAGED_DENSE_COMP_DATA(CSurfaceInteractionHandle, 16)
}; 


struct CTextureSamplerIsotropic
{	 
	DECL_MANAGED_DENSE_COMP_DATA(CTextureSamplerIsotropic, 1)
};

struct CTextureSamplerAnistropic
{
	float maxAnisotropy;
	HEntity filterTable;
	DECL_MANAGED_DENSE_COMP_DATA(CTextureSamplerAnistropic, 1)
};

struct CFilterTableGaussing
{
	std::vector<float, AllocatorAligned2<float>> weights;
	uint32_t size;
	DECL_MANAGED_DENSE_COMP_DATA(CFilterTableGaussing, 1)
};

class JobFilterTableGaussing: public JobParallazible
{
	uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return min(queryComponentsGroup<CFilterTableGaussing>().size(), nWorkThreads);
	}

	void update(WECS* ecs, uint8_t iThread) override;
};


class STextureSampler
{
public:

	template<typename T>
	static T triangle(const DPoint2f& st,
						 const Texture2DBase<T>& tex,
						 uint32_t mip)
	{
		uint32_t nLevels = tex.mips.size();
		mip = wml::clamp(mip, 0, nLevels - 1);
		float s = st[0] * tex.resolutions[mip].x() - 0.5f;
		float t = st[1] * tex.resolutions[mip].y() - 0.5f;
		int s0 = std::floor(s), t0 = std::floor(t);
		s0 = max(s0, 0);
		t0 = max(t0, 0);


		float ds = s - s0, dt = t - t0;

		int s0n = min(s0 + 1, tex.resolutions[mip].x()-1);
		int t0n = min(t0 + 1, tex.resolutions[mip].y() - 1);
		s0n = max(s0n, 0);
		t0n = max(t0n, 0);
		return	tex.texel(s0, t0, mip)*(1 - ds) * (1 - dt) +
			tex.texel(s0, t0n, mip)*(1 - ds) * dt +
			tex.texel(s0n, t0, mip)*ds * (1 - dt) +
			tex.texel(s0n, t0n, mip)*ds * dt;
	}
};

class STextureSampleAnisotropic
{
public:
	template<typename T>
	static T sample(
		const CTextureMappedPoint& st,
		const CTextureSamplerAnistropic& sampler,
		const CFilterTableGaussing& filterTable,
		const Texture2DBase<T>& tex)
	{
		DVector2f dst0 = st.dstdx;
		DVector2f dst1 = st.dstdy;
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
			return STextureSampler::triangle(st.p, tex, 0);
		}


		uint32_t nLevels = tex.mips.size();
	//	float lod = max(0.0f, (nLevels-1) - max(log2(minorLength*tex.resolutions[0].x()) - 2, 0.0));
		float lod = max(0.0f, nLevels - 1 + log2(max(minorLength, 0)));
		int ilod = std::floor(lod);
		if (ilod >= nLevels - 1)
		{
			return ewa(st, tex, filterTable, nLevels - 1);
		}
		else
		{
			return wml::lerp(ewa(st, tex, filterTable, ilod),
							 ewa(st, tex, filterTable, ilod + 1), lod - ilod);
		}
	}

	template<typename T>
	static T ewa(const CTextureMappedPoint& p,
					const Texture2DBase<T>& tex,
					const CFilterTableGaussing& filter,
					uint32_t mip)
	{
		uint32_t nLevels = tex.mips.size();

		DVector2f st = p.p;
		DVector2f dst0 = p.dstdx;
		DVector2f dst1 = p.dstdy;


		st = st*tex.resolutions[mip] - 0.5f;
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
		float uSqrt = std::sqrt(det * C), vSqrt = std::sqrt(A * det);

		DVector2f uvSqrt(C, A);
		uvSqrt *= det;
		uvSqrt = sqrt(uvSqrt);
		uvSqrt *= 2 * invDet;

		DVector2f st0 = clamp(st - uvSqrt, 0.0, tex.resolutions[mip].x()-1.0);
		DVector2f st1 = clamp(st + uvSqrt, 0.0, tex.resolutions[mip].x() - 1.0);

		int s0 = std::ceil(st0[0]);
		int t0 = std::ceil(st0[1]);

		int s1 = std::floor(st1[0]);
		int t1 = std::floor(st1[1]);

		T sum(0.f);
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
					int index = min((int)(r2 * filter.size),
										 filter.size - 1);
					float weight = filter.weights[index];
					sum += tex.texel(is, it, mip) * weight;
					sumWts += weight;
				}

			}
		}
		return sum / sumWts;
	}
};



class STextureSamplerIsotropic
{

public:

	template<typename T>
	static T sample(
		   const CTextureMappedPoint& st,
		   const Texture2DBase<T>& tex) 
	{
		float width = 2 * max(max(st.dstdx[0], st.dstdx[1]),
								   max(st.dstdy[0], st.dstdy[1]));

		uint32_t nLevels = tex.mips.size();
		float level = nLevels - 1 + log2(max(width, 0));
		if (level < 0)
		{
			return STextureSampler::triangle(st.p, tex, 0);
		}
		else if (level >= nLevels - 1)
		{
			return tex.texel(0, 0, nLevels - 1);
		}
		else
		{
			int iLevel = std::floor(level);
			float delta = level - iLevel;
			return lerp(STextureSampler::triangle(st.p, tex, iLevel), STextureSampler::triangle(st.p, tex, iLevel + 1), delta);
		}
	}
};

class SLoadTexture
{

public:
	template<typename T, typename CompT>
	static void load(std::map<std::string, HEntity>& textures, CTextureBindingBase<CompT>& textureInfo)
	{
		std::map<std::string, HEntity>::const_iterator it = textures.find(textureInfo.filename);
		if (it != textures.cend())
		{
			textureInfo.tex = it->second;
			return;
		}


		DPoint2i resolutions;
		std::vector<T, AllocatorAligned2<T>> texels = readImagePNG(textureInfo.filename, resolutions);

		for (uint32_t i = 0; i < texels.size(); i++)
		{
			for (uint32_t k = 0; k < texels[i].size(); k++)
			{
				texels[i][k] = gammaCorrectInv(texels[i][k]);
			}
		}

		assert(!texels.empty());

		HEntity hTexture = ecs->createEntity();
		CompT texRGB = generateMIPS<T, CompT>(std::move(texels), resolutions);
		ecs->addComponent<CompT>(hTexture, std::move(texRGB));

		textureInfo.tex = hTexture;
		textures[textureInfo.filename] = hTexture;
	}

	template<typename T, typename CompT>
	static CompT generateMIPS(std::vector<T, AllocatorAligned2<T>> texels, DPoint2i resolution)
	{
		uint16_t nLevels = log2(max(resolution[0], resolution[1])) - 1;
		CompT tex;
		tex.resolutions.resize(nLevels);
		tex.mips.resize(nLevels);
		tex.mipNBlocks.resize(nLevels);

		tex.resolutions[0] = std::move(resolution);
		tex.mipNBlocks[0] = tex.resolutions[0] / CompT::blockSize;
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
			tex.mipNBlocks[i] = res / CompT::blockSize;

			std::vector<T, AllocatorAligned2<T>>& mip = tex.mips[i];
			for (uint32_t y = 0; y < res.y() - 1; y++)
			{
				for (uint32_t x = 0; x < res.x() - 1; x++)
				{
					tex.texel(x, y, i) = (tex.texel(2 * x, 2 * y, i - 1) + tex.texel(2 * x + 1, 2 * y, i - 1) +
										  tex.texel(2 * x, 2 * y + 1, i - 1) + tex.texel(2 * x + 1, 2 * y + 1, i - 1))*0.25;
				}

				tex.texel(res.x() - 1, y, i) = (tex.texel(tex.resolutions[i - 1].x() - 1, 2 * y, i - 1) +
												tex.texel(tex.resolutions[i - 1].x() - 1, 2 * y + 1, i - 1))*0.5;
			}

			for (uint32_t x = 0; x < res.x() - 1; x++)
			{
				tex.texel(x, res.y() - 1, i) = (tex.texel(2 * x, tex.resolutions[i - 1].y() - 1, i - 1) +
												tex.texel(2 * x + 1, tex.resolutions[i - 1].y() - 1, i - 1))*0.5;
			}

			tex.texel(res.x() - 1, res.y() - 1, i) = tex.texel(tex.resolutions[i - 1].x() - 1, tex.resolutions[i - 1].y() - 1, i - 1);
		}
		return tex;
	}

	template<typename T, typename CompT>
	static void load3D(std::map<std::string, HEntity>& textures, CTextureBindingBase<CompT>& textureInfo)
	{
		std::map<std::string, HEntity>::const_iterator it = textures.find(textureInfo.filename);
		if (it != textures.cend())
		{
			textureInfo.tex = it->second;
			return;
		}


		DPoint2i resolutions;
		std::vector<T, AllocatorAligned2<T>> texels = readImagePNG(textureInfo.filename, resolutions);

		DPoint3i resolutionReal(pow(resolutions.area(), 1.0 / 3.0));

		for (uint32_t i = 0; i < texels.size(); i++)
		{
			for (uint32_t k = 0; k < texels[i].size(); k++)
			{
				texels[i][k] = gammaCorrectInv(texels[i][k]);
			}
		}

		assert(!texels.empty());

		HEntity hTexture = ecs->createEntity();
		CompT texRGB = generateMIPS<T, CompT>(std::move(texels), resolutionReal);
		ecs->addComponent<CompT>(hTexture, std::move(texRGB));

		textureInfo.tex = hTexture;
		textures[textureInfo.filename] = hTexture;
	}

	template<typename T, typename CompT>
	static CompT generateMIPS3D(std::vector<T, AllocatorAligned2<T>> texels, DPoint3i resolution)
	{
		uint16_t nLevels = 1;
		CompT tex;
		tex.resolutions.resize(nLevels);
		tex.mips.resize(nLevels);
		tex.mipNBlocks.resize(nLevels);

		tex.resolutions[0] = std::move(resolution);
		tex.mipNBlocks[0] = tex.resolutions[0] / CompT::blockSize;
		tex.mips[0].resize(tex.resolutions[0].area());
		for (uint32_t y = 0; y < tex.resolutions[0].y(); y++)
		{
			for (uint32_t x = 0; x < tex.resolutions[0].x(); x++)
			{
				for (uint32_t z = 0; z < tex.resolutions[0].z(); z++)
				{
					tex.texel(x, y, z, 0) = std::move(texels[x + y * tex.resolutions[0].x() + z*tex.resolutions[0].x()*tex.resolutions[0].y()]);
				}
			}
		}

		return tex;
	}



	template<typename T>
	static std::vector<T, AllocatorAligned2<T>> readImagePNG(const std::string &name, DPoint2i& resolution)
	{
		unsigned char *rgb;
		unsigned w, h;
		unsigned int error = lodepng_decode24_file(&rgb, &w, &h, name.c_str());
		assert(error == 0);
		resolution.x() = w;
		resolution.y() = h;

		std::vector<T, AllocatorAligned2<T>> ret(resolution.x()*resolution.y());
		unsigned char *src = rgb;
		for (unsigned int y = 0; y < h; ++y)
		{
			for (unsigned int x = 0; x < w; ++x, src += 3)
			{
				T tmp;
				for (uint32_t k = 0; k < tmp.size(); k++)
				{
					tmp[k] = src[k];
				}

				src += k;

				ret[y * resolution.x() + x] = tmp / 255.0f;
			}
		}

		free(rgb);
		return ret;
	} 



	static float gammaCorrect(float value)
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

	static inline float gammaCorrectInv(float value)
	{
		if (value <= 0.04045f)
			return value * 1.f / 12.92f;
		return std::pow((value + 0.055f) * 1.f / 1.055f, 2.4f);
	}



};

class JobLoadTexture2DRGB: public Job
{
public:
	static std::map<std::string, HEntity> textures;
	void update(WECS* ecs) override;
};

class JobLoadTexture2DR : public Job
{
public:
	static std::map<std::string, HEntity> textures;
	void update(WECS* ecs) override;
};

class JobLoadTexture3DRGB : public Job
{
public:
	static std::map<std::string, HEntity> textures;
	void update(WECS* ecs) override;
};

class JobLoadTexture3DR : public Job
{
public:
	static std::map<std::string, HEntity> textures;
	void update(WECS* ecs) override;
};

WPBR_END

