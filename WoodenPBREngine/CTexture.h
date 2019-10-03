#pragma once

#include "pch.h"
#include <map>

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

template<typename T>
class TextureConst
{
	T sample(const CSurfaceInteraction& si) const
	{
		return value;
	}

	T value;
};

template<typename T>
struct TextureBase
{
	std::string filename;
	std::vector<std::vector<T, AllocatorAligned2<T>>> mips;
	std::vector<DPoint2i, AllocatorAligned2<DPoint2i>> resolutions;
	std::vector<DPoint2i, AllocatorAligned2<DPoint2i>> mipNBlocks;

	static constexpr uint8_t blockSize = 4;
	uint32_t iTexel(const DPoint2i& p, uint8_t mip) const
	{
		const DPoint2i& nBlocks = mipNBlocks[mip];
		constexpr uint8_t blockArea = blockSize * blockSize;
		DPoint2i block = p / blockSize;

		uint32_t iBlock = block.x() + block.y()*nBlocks.x();
		uint32_t i = iBlock * blockArea + (p.x() % blockSize) + (p.y() % blockSize)*blockSize;
		return i;
	}

	const T& texel(const DPoint2i& p, uint8_t mip) const
	{
		return mips[mip][iTexel(p, mip)];
	}

	const T& texel(uint32_t x, uint32_t y, uint8_t mip) const
	{
		return mips[mip][iTexel(DPoint2i(x, y), mip)];
	}

	T& texel(const DPoint2i& p, uint8_t mip)
	{
		return mips[mip][iTexel(p, mip)];
	}

	T& texel(uint32_t x, uint32_t y, uint8_t mip)
	{
		return mips[mip][iTexel(DPoint2i(x, y), mip)];
	}

};


struct CTextureRGB : public TextureBase<RGBSpectrum>
{
	DECL_MANAGED_DENSE_COMP_DATA(CTextureRGB, 16)
}; 

struct CTextureR : public TextureBase<float>
{

	DECL_MANAGED_DENSE_COMP_DATA(CTextureR, 16)
};





template<typename T>
struct CTextureBindingBase
{

	std::string filename;
	HEntity tex;

	const T& getTex(WECS* ecs) const
	{
		return ecs->getComponent<T>(tex);
	}
};

struct CTextureBindingRGB: public CTextureBindingBase<CTextureRGB>
{
	DECL_MANAGED_DENSE_COMP_DATA(CTextureBindingRGB, 16)
}; 

struct CTextureBindingR: public CTextureBindingBase<CTextureR>
{
	DECL_MANAGED_DENSE_COMP_DATA(CTextureBindingR, 16)
};



class STextureBindingRGB
{
public:
	static HEntity create(const std::string& filename)
	{
		CTextureBindingRGB tex;
		tex.filename = filename;

		MEngine& mEngine = MEngine::getInstance();
		HEntity h = mEngine.createEntity();
		mEngine.addComponent<CTextureBindingRGB>(h, std::move(tex));
		return h;
	}
};

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
						 const TextureBase<T>& tex,
						 uint32_t mip)
	{
		uint32_t nLevels = tex.mips.size();
		mip = wml::clamp(mip, 0, nLevels - 1);
		float s = st[0] * tex.resolutions[mip].x() - 0.5f;
		float t = st[1] * tex.resolutions[mip].y() - 0.5f;
		int s0 = std::floor(s), t0 = std::floor(t);
		float ds = s - s0, dt = t - t0;

		int s0n = min(s0 + 1, tex.resolutions[mip].x()-1);
		int t0n = min(t0 + 1, tex.resolutions[mip].y() - 1);
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
		const TextureBase<T>& tex)
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
		float lod = max(0.0f, (nLevels-1) - max(log2(minorLength) - 2, 0.0));
		int ilod = std::floor(lod);
		if (ilod == nLevels - 1)
		{
			return ewa(st, tex, filterTable, ilod);
		}
		else
		{
			return wml::lerp(ewa(st, tex, filterTable, ilod),
							 ewa(st, tex, filterTable, ilod + 1), lod - ilod);
		}
	}

	template<typename T>
	static T ewa(const CTextureMappedPoint& p,
					const TextureBase<T>& tex,
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
		   const TextureBase<T>& tex) 
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

class JobLoadTextureRGB: public Job
{
public:
	static std::map<std::string, HEntity> textures;
	void update(WECS* ecs) override;

	CTextureRGB generateMIPs(std::vector<RGBSpectrum, AllocatorAligned2<RGBSpectrum>> texels, DPoint2i resolution);
	
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

	//static unsigned int  lodepng_decode24_file(unsigned char** rgb, unsigned* w, unsigned* h, const char* name)
	//{
	//	static_assert(false || "Not implemented yet");
	//}

	static std::vector<RGBSpectrum, AllocatorAligned2<RGBSpectrum>> readImagePNG(const std::string &name, DPoint2i& resolution);
};



WPBR_END

