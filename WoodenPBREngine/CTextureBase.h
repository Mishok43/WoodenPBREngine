
#pragma once
#include "pch.h"
#include "MEngine.h"

WPBR_BEGIN

template<typename T>
struct Texture2DBase
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

template<typename T>
struct Texture3DBase
{
	std::string filename;
	std::vector<std::vector<T, AllocatorAligned2<T>>> mips;
	std::vector<DPoint3i, AllocatorAligned2<DPoint3i>> resolutions;
	std::vector<DPoint3i, AllocatorAligned2<DPoint3i>> mipNBlocks;

	static const DVector3i blockSize = DVector3i(2, 2, 4); // 2*2*4 = 16 elements per block -> 16*4 = 64 bytes per float block = cache line on average
	uint32_t iTexel(const DPoint2i& p, uint8_t mip) const
	{
		const DPoint3i& nBlocks = mipNBlocks[mip];
		constexpr uint32_t blockArea = blockSize.area();
		DPoint3i block = p / blockSize;

		uint32_t iBlock = block.x() + block.y()*nBlocks.x() + block.z()*(nBlocks.x()*nBlocks.y());
		uint32_t i = iBlock * blockArea + (p.x() % blockSize.x()) + (p.y() % blockSize.y())*blockSize.x() + (p.z() % blockSize.z())*blockSize.x()*blockSize.y();
		return i;
	}

	const T& texel(const DPoint3i& p, uint8_t mip) const
	{
		mip = 0;
		return mips[mip][iTexel(p, mip)];
	}

	const T& texel(uint32_t x, uint32_t y, uint32_t z, uint8_t mip) const
	{
		mip = 0;
		return mips[mip][iTexel(DPoint2i(x, y, z), mip)];
	}

	T& texel(const DPoint3i& p, uint8_t mip)
	{
		mip = 0;
		return mips[mip][iTexel(p, mip)];
	}

	T& texel(uint32_t x, uint32_t y, uint32_t z, uint8_t mip)
	{
		mip = 0;
		return mips[mip][iTexel(DPoint3i(x, y, z), mip)];
	}
};

struct CTexture2DRGB : public Texture2DBase<DVectorPacked<float, 3>>
{
	DECL_MANAGED_DENSE_COMP_DATA(CTexture2DRGB, 4)
};

struct CTexture2DR : public Texture2DBase<DVectorPacked<float, 1>>
{
	DECL_MANAGED_DENSE_COMP_DATA(CTexture2DR, 4)
};

struct CTexture3DRGB : public Texture3DBase<DVectorPacked<float, 3>>
{
	DECL_MANAGED_DENSE_COMP_DATA(CTexture3DRGB, 4)
};

struct CTexture3DR : public Texture3DBase<DVectorPacked<float, 1>>
{
	DECL_MANAGED_DENSE_COMP_DATA(CTexture3DR, 4)
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


struct CTextureBinding2DRGB : public CTextureBindingBase<CTexture2DRGB>
{
	DECL_MANAGED_DENSE_COMP_DATA(CTextureBinding2DRGB, 16)
};

struct CTextureBinding2DR : public CTextureBindingBase<CTexture2DR>
{
	DECL_MANAGED_DENSE_COMP_DATA(CTextureBinding2DR, 16)
};

struct CTextureBinding3DRGB : public CTextureBindingBase<CTexture3DRGB>
{
	DECL_MANAGED_DENSE_COMP_DATA(CTextureBinding2DRGB, 16)
};

struct CTextureBinding3DR : public CTextureBindingBase<CTexture3DR>
{
	DECL_MANAGED_DENSE_COMP_DATA(CTextureBinding2DR, 16)
};



class STextureBindingRGB
{
public:
	static HEntity create(const std::string& filename)
	{
		CTextureBinding2DRGB tex;
		tex.filename = filename;

		MEngine& mEngine = MEngine::getInstance();
		HEntity h = mEngine.createEntity();
		mEngine.addComponent<CTextureBinding2DRGB>(h, std::move(tex));
		return h;
	}
};


WPBR_END