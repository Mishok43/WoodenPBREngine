#pragma once
#include "pch.h"
#include "MEngine.h"
#include "CBounds.h"
#include "CSTriangle.h"
#include "CRay.h"
#include "CSufraceInteraction.h"
#include "CCentroid.h"
#include "WoodenAllocators/AllocatorLinear.h"
#include "WoodenECS/Job.h"
#include "CSSphere.h"
#include <algorithm>
#include <array>
WPBR_BEGIN


struct LinearBVHNode
{
	union
	{
		uint32_t shapeOffset;
		uint32_t secondChildOffset;
	};
	uint32_t nShapes; // 0 -> interior node
	uint8_t axis;
};

struct CLBVHTree
{
	std::vector<LinearBVHNode, AllocatorAligned2<LinearBVHNode>> nodes;
	std::vector<DBounds3f, AllocatorAligned2<DBounds3f>> nodesBounds;
	std::vector<HEntity, AllocatorAligned2<HEntity>> shapesEntities;

	DECL_MANAGED_DENSE_COMP_DATA(CLBVHTree, 1)
}; 

class JobProcessRayCastsResults : public JobParallazible
{
	constexpr static uint32_t sliceSize = 32;

	virtual uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return min(nWorkThreads, (queryComponentsGroup<CRayCast>().size()+sliceSize-1)/sliceSize);
	}

	virtual void update(WECS* ecs, uint8_t iThread) override;

	virtual void finish(WECS* ecs) override;
};


class JobCollisionFinish : public Job
{
	void update(WECS* ecs) override;

	void finish(WECS* ecs) override;
};

class JobProcessRayCasts: public JobParallazible
{
	virtual uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return min(nWorkThreads, queryComponentsGroup<CRayCast>().size());
	}

	void update(WECS* ecs, uint8_t iThread) override;
};

struct BVHBuildNode
{
	uint32_t iStartShape;
	uint32_t nShapes;
	DBounds3f* bounds;
	BVHBuildNode* children[2];
	uint8_t splitAxis;

	void initAsInterior(uint8_t axis, BVHBuildNode* child0, BVHBuildNode* child1, DBounds3f* bounds)
	{
		assert(axis < 3);
		children[0] = child0;
		children[1] = child1;
		splitAxis = axis;
		this->bounds = bounds;
		*this->bounds = DBounds3f(*child0->bounds, *child1->bounds);
		nShapes = 0;
	}

	void initAsLeaf(uint32_t iStartShapeBounds, uint32_t nShapeBounds, DBounds3f* bounds)
	{
		this->nShapes = nShapeBounds;
		this->iStartShape = iStartShapeBounds;
		this->bounds = bounds;
		children[0] = children[1] = nullptr;
	}
};

struct CentroidMortonCode
{
	uint32_t value;
	uint32_t index;
};

struct LBVHSubTreeBuilder : public BVHBuildNode
{
	BVHBuildNode* nodes;
	DBounds3f* nodesBounds;
	uint32_t nNodes;
	uint8_t iBucket;
};


struct CLBVHTreeBuilder
{
	CLBVHTreeBuilder() = default;

	DBounds3f boundsCentroid;
	std::vector<CentroidMortonCode, AllocatorAligned2<CentroidMortonCode>> shapeMortonCodes;
	std::vector<CBounds, AllocatorAligned2<CBounds>> shapeBounds;
	std::vector<HEntity, AllocatorAligned2<HEntity>> shapeEntities;
	std::vector<LBVHSubTreeBuilder, AllocatorAligned2<LBVHSubTreeBuilder>> subTreeBuilders;
	std::vector<BVHBuildNode, AllocatorAligned2<BVHBuildNode>> buildNodes;
	std::vector<DBounds3f, AllocatorAligned2<DBounds3f>> buildNodesBounds;
	std::vector<BVHBuildNode, AllocatorAligned2<BVHBuildNode>> sahBuildNodes;
	std::vector<DBounds3f, AllocatorAligned2<DBounds3f>> sahBuildNodesBounds;

	uint32_t nSAHNodesTotal = 0;
	uint32_t nSubTreeNodesTotal = 0;
	DECL_MANAGED_DENSE_COMP_DATA(CLBVHTreeBuilder, 1)
}; 

class JobGenerateShapesCentroidBound: public Job
{
	void update(WECS* ecs) override;
};

class JobGenerateShapesMortonCode : public JobParallazible
{
	constexpr static uint32_t sliceSize = 512;

	virtual uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		ComponentsGroup<CCentroid, CBounds> shapesData = queryComponentsGroup<CCentroid, CBounds>();
		return min((shapesData.size<CCentroid>()+sliceSize-1)/sliceSize, nWorkThreads);
	}


	void update(WECS* ecs, uint8_t iThread) override;

	uint32_t leftShift3(uint32_t x);
};

class JobSortsShapesByMortonCode : public Job
{
	void update(WECS* ecs) override;
};

class JobEmitLBVH : public JobParallazible
{
	virtual uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override;

	BVHBuildNode* emitLBVH(CLBVHTreeBuilder& treeBuilder, BVHBuildNode* nodes, DBounds3f* nodesBounds, uint32_t& boundsOffset,
						   uint32_t iStartShape, uint32_t nShapes, int bitIndex, uint32_t& totalNodes);

	uint32_t maxPrimitiveInNode = 10;
};

class JobBuildUpperSAH: public Job
{

	void update(WECS* ecs) override;

	uint32_t flattenBVHTree(BVHBuildNode* node, CLBVHTree& tree, uint32_t& offset);
	BVHBuildNode* buildUpperSAH(CLBVHTreeBuilder& treeBuilder, uint32_t iStart, uint32_t iLast);
};


WPBR_END