#pragma once
#include "pch.h"
#include "MEngine.h"
#include "CBounds.h"
#include "CSTriangle.h"
#include "CScene.h"
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
		uint16_t shapeOffset;
		uint16_t secondChildOffset;
	};
	uint16_t nShapes; // 0 -> interior node
	uint8_t axis;
};

struct CLBVHTree
{
	std::vector<LinearBVHNode, AllocatorAligned<LinearBVHNode>> nodes;
	std::vector<DBounds3f, AllocatorAligned<DBounds3f>> nodesBounds;
	std::vector<HEntity, AllocatorAligned<HEntity>> shapesEntities;

	DECL_MANAGED_DENSE_COMP_DATA(CLBVHTree, 1)
}; DECL_OUT_COMP_DATA(CLBVHTree)

class JobProcessRayCastsResults : public JobParallazible
{
	constexpr static uint16_t sliceSize = 32;

	virtual uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		ComponentsGroup<CRayCast> rayCast = queryComponentsGroup<CRayCast>();
		return std::min(nWorkThreads, (rays.size<CRay>()+sliceSize-1)/sliceSize);
	}

	virtual void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t nRaysCasts = queryComponentsGroup<CRayCast>().size<CRayCast>();

		uint32_t slice = (nRaysCasts + getNumThreads()-1) /getNumThreads();
		uint32_t iStart = iThread * slice;
		ComponentsGroupSlice<CRay> rayCasts = queryComponentsGroupSlice<CRay>(Slice(iStart, slice));
		for_each([ecs](HEntity hEntity, CRayCast& rayCast)
		{
			HEntity interactionEntity;
			float tHitMin = std::numeric_limits<float>::infinity();
			for (uint16_t i = 0; i < rayCast.interactionEntities.size(); i++)
			{
				HEntity hInteractionEntity = rayCast.interactionEntities[i];
				const CInteractionRequest& interactionRequest = 
					ecs->getComponent<CInteractionRequest>(hInteractionEntity);

				if (interactionRequest.tHitResult == 0.0)
				{
					continue;
				}

				if (tHitMin > interactionRequest.tHitResult)
				{
					tHitMin = interactionRequest.tHitResult;
					interactionEntity = hInteractionEntity;
				}
			}

			if (tHitMin == std::numeric_limits<float>::infinity())
			{
				rayCast.hCollision = INVALID_HANDLE;
			}
			else
			{
				rayCast.hCollision = interactionEntity;
				if (rayCast.bSurfInteraction)
				{
					ecs->addComponent<CFullInteractionRequest>(interactionEntity);
				}
				else
				{
					CInteraction interaction;
					interaction.p = rayCast.ray(tHitMin);
					interaction.time = tHitMin;
					interaction.wo = -rayCast.ray.dir;

					ecs->addComponent<CInteraction>(hEntity, std::move(interaction));
				}
			}
		}, rayCasts);
	}

	virtual void finish(WECS* ecs) override
	{
		ecs->deleteEntitiesOfComponents<CInteractionRequest>();
		ecs->clearComponents<CInteractionSphere>();
		ecs->clearComponents<CInteractionTriangle>();
		ecs->clearComponents<CFullInteractionRequest>();
		ecs->clearComponents<CInteractionRequest>();
		ecs->clearComponents<CRayCast>();
	}
};

class JobProcessRayCasts: public JobParallazible
{
	virtual uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		ComponentsGroup<CRayCast> rayCast = queryComponentsGroup<CRayCast>();
		return std::min(nWorkThreads, rays.size<CRay>());
	}

	virtual void update(WECS* ecs, uint8_t iThread) override
	{
		uint32_t nRays = queryComponentsGroup<CRayCast>().size<CRayCast>();

		uint32_t slice = (nRays+nThreads-1) /getNumThreads();
		uint32_t iStart = iThread * slice;
		ComponentsGroupSlice<CRay> rays = queryComponentsGroupSlice<CRay>(Slice(iStart, slice));
		ComponentsGroup<CLBVHTree> trees = queryComponentsGroup<CLBVHTree>();
		const CLBVHTree& tree = trees.getRawData<CLBVHTree>()[0];
		for_each([tree, ecs](HEntity hEntity, CRayCast& rayCast)
		{

			const DRayf ray = rayCast.ray;
			DVector3f dirInv = DVector3f(1.0) / ray.dir;
			int dirIsNeg[3] = { dirInv.x() < 0, dirInv.y() < 0, dirInv.z() < 0 };
			int toVisitOffset = 0, currentNodeIndex = 0;
			int nodesToVisit[64];
			while (true)
			{
				const LinearBVHNode& node = tree.nodes[currentNodeIndex];
				const DBounds3f& nodeBounds = tree.nodesBounds[currentNodeIndex];
				if (nodeBounds.intersect(ray, dirInv, dirIsNeg))
				{
					if (node.nShapes > 0)
					{
						for (uint16_t i = 0; i < node.nShapes; ++i)
						{
							HEntity shapeEntity = tree.shapesEntities[node.shapeOffset + i];

							CInteractionRequest request;
							request.rayCastEntity = hEntity;

							HEntity eRequest = ecs->createEntity();
							ecs->addComponent<CInteractionRequest>(eRequest, std::move(eRequest));
							// CRUNCH!
							if (shapeEntity.hasComponent<CTriangle>())
							{
								CInteractionTriangle interactionTriangle;
								interactionTriangle.hTriangle = shapeEntity;
								ecs->addComponent<CInteractionTriangle>(eRequest, std::move(interactionTriangle));
							}
							else if(shapeEntity.hasComponent<CSphere>())
							{
								CInteractionSphere interactionSphere;
								interactionSphere.hSphere = shapeEntity;
								ecs->addComponent<CInteractionSphere>(eRequest, std::move(interactionSphere));
							}
						}
						if (toVisitOffset == 0)
						{
							break;
						}
						currentNodeIndex = nodesToVisit[--toVisitOffset];
					}
					else
					{
						if (dirIsNeg[node.axis])
						{
							nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
							currentNodeIndex = node.secondChildOffset;
						}
						else
						{
							nodesToVisit[toVisitOffset++] = node.secondChildOffset;
							currentNodeIndex = currentNodeIndex + 1;
						}

					}
				}
				else
				{
					if (toVisitOffset == 0)
					{
						break;
					}
					currentNodeIndex = nodesToVisit[--toVisitOffset];
				}

			}

		}, rays);
	}
};

struct BVHBuildNode
{
	uint16_t iStartShape;
	uint16_t nShapes;
	DBounds3f* bounds;
	BVHBuildNode* children[2];
	uint8_t splitAxis;

	void initAsInterior(uint8_t axis, BVHBuildNode* child0, BVHBuildNode* child1, DBounds3f* bounds)
	{
		children[0] = child0;
		children[1] = child1;
		splitAxis = axis;
		this->bounds = bounds;
		*this->bounds = DBounds3f(*child0->bounds, *child1->bounds);
		nShapes = 0;
	}

	void initAsLeaf(uint16_t iStartShapeBounds, uint16_t nShapeBounds, DBounds3f* bounds)
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
	uint16_t index;
};

struct CLBVHTreeBuilder
{
	CLBVHTreeBuilder() = default;

	DBounds3f boundsCentroid;
	std::vector<CentroidMortonCode, AllocatorAligned<CentroidMortonCode>> shapeMortonCodes;
	std::vector<CBounds, AllocatorAligned<CBounds>> shapeBounds;
	std::vector<HEntity, AllocatorAligned<HEntity>> shapeEntities;
	std::vector<LBVHSubTreeBuilder, AllocatorAligned<LBVHSubTreeBuilder>> subTreeBuilders;
	std::vector<BVHBuildNode, AllocatorAligned<BVHBuildNode>> buildNodes;
	std::vector<DBounds3f, AllocatorAligned<DBounds3f>> buildNodesBounds;
	std::vector<BVHBuildNode, AllocatorAligned<BVHBuildNode>> sahBuildNodes;
	std::vector<DBounds3f, AllocatorAligned<DBounds3f>> sahBuildNodesBounds;

	uint32_t nSAHNodesTotal;
	uint32_t nSubTreeNodesTotal;
	DECL_MANAGED_DENSE_COMP_DATA(CLBVHTreeBuilder, 1)
}; DECL_OUT_COMP_DATA(CLBVHTreeBuilder)

struct LBVHSubTreeBuilder : public BVHBuildNode
{
	BVHBuildNode* nodes;
	DBounds3f* nodesBounds;
	uint16_t nNodes;
	uint8_t iBucket;
};

class JobGenerateShapesCentroidBound: public Job
{
	void update(WECS* ecs) override
	{
		ComponentsGroup<CCentroid> centroids = queryComponentsGroup<CCentroid>();
		uint16_t nShapes = centroids.size<CCentroid>();

		CLBVHTreeBuilder treeBuilder;
		treeBuilder.shapeBounds.reserve(nShapes);
		treeBuilder.shapeMortonCodes.reserve(nShapes);
		treeBuilder.shapeEntities.reserve(nShapes);

		for_each([&treeBuilder](HEntity hEntity, const CCentroid& eCentroid)
		{
			treeBuilder.boundsCentroid = DBounds3f(treeBuilder.boundsCentroid, eCentroid);
		}, centroids);

		HEntity hEntity = ecs->createEntity();
		ecs->addComponent<CLBVHTreeBuilder>(hEntity, std::move(treeBuilder));
	}
};

class JobGenerateShapesMortonCode : public JobParallazible
{
	constexpr static uint16_t sliceSize = 512;

	void update(WECS* ecs, uint8_t iThread) override
	{
		ComponentsGroup<CLBVHTreeBuilder> treeBuilderData = queryComponentsGroup<CLBVHTreeBuilder>();

		Slice slice = Slice(iThread*sliceSize, sliceSize);
		ComponentsGroupSlice<CCentroid, CBounds> shapesData = queryComponentsGroupSlice<CCentroid, CBounds>(slice);
	
		for_each([this, ecs, &shapesData, iThread](HEntity, CLBVHTreeBuilder& treeBuilder)
		{
			uint16_t iShape = iThread;
			for_each([this, ecs, &shapesData, &treeBuilder, &iShape]
			(HEntity hEntity, const CCentroid& eCentroid, const CBounds& eBounds)
			{
				constexpr int32_t mortonBits = 10;
				constexpr int32_t mortonScale = 1 << mortonBits;
				DPoint3f centroidN = treeBuilder.boundsCentroid.getLerpFactors(eCentroid);
				uint32_t mortonCode = 
					(leftShift3(centroidN.z) << 2) |
					(leftShift3(centroidN.y) << 1) |
					leftShift3(centroidN.x);

				CentroidMortonCode code;
				code.value = mortonCode;
				code.index = iShape;
				
				treeBuilder.shapeMortonCodes[iShape] = std::move(code);
				treeBuilder.shapeBounds[iShape] = eBounds;
				treeBuilder.shapeEntities[iShape] = hEntity;
				iShape++;
			}, shapesData);
		}, treeBuilderData);
	}

	virtual uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		ComponentsGroup<CCentroid, CBounds> shapesData = queryComponentsGroup<CCentroid, CBounds>();
		return std::min((shapesData.size<CCentroid>()+sliceSize-1)/sliceSize, nWorkThreads);
	}

	uint32_t leftShift3(uint32_t x)
	{
		if (x == (1 << 10)) --x;
		x = (x | (x << 16)) & 0b00000011000000000000000011111111;
		x = (x | (x << 8)) & 0b00000011000000001111000000001111;
		x = (x | (x << 4)) & 0b00000011000011000011000011000011;
		x = (x | (x << 2)) & 0b00001001001001001001001001001001;
		return x;
	}
};

class JobSortsShapesByMortonCode : public Job
{
	void update(WECS* ecs) override
	{
		ComponentsGroup<CLBVHTreeBuilder> treeBuilders = queryComponentsGroup<CLBVHTreeBuilder>();
		for_each([this, ecs](CLBVHTreeBuilder& treeBuilder)
		{
			AllocatorLinear allocTemp = getAllocatorTemp();
			uint32_t nMortonCodes = treeBuilder.shapeEntities.size();

			CentroidMortonCode* mortonCodes = treeBuilder.shapeMortonCodes.data();
			CentroidMortonCode* mortonCodesTmp = (CentroidMortonCode*)allocTemp.allocMem(sizeof(CentroidMortonCode)*nMortonCodes);

			constexpr int bitsPerPass = 6;
			constexpr int nBits = 30;
			constexpr int nPasses = nBits / bitsPerPass;
			constexpr int nBuckets = 1 << 6;
			constexpr int bitMask = nBuckets - 1;
			for (uint8_t iPass = 0; iPass < nPasses; iPass++)
			{
				CentroidMortonCode* in = (iPass & 1) ? mortonCodesTmp : mortonCodes;
				CentroidMortonCode* out = (iPass & 1) ? mortonCodes : mortonCodesTmp;
				int8_t lowBit = iPass * bitsPerPass;

				std::array<uint16_t, nBuckets> counters;
				std::memset(counters.data(), 0, sizeof(uint16_t)*nBuckets);

				for (uint16_t i = 0; i < nMortonCodes; i++)
				{
					uint8_t iCounter = (in[i].value >> lowBit) & bitMask;
					++counters[iCounter];
				}


				// convert counters to offsets
				uint16_t total = 0;
				for (uint16_t i = 0; i < nBuckets; i++)
				{
					uint16_t tmp = counters[i];
					counters[i] = total;
					total += tmp;
				}

				for (uint16_t i = 0; i < nMortonCodes; i++)
				{
					uint8_t iCounter = (in[i].value >> lowBit) & bitMask;
					uint16_t sortedPos = counters[iCounter]++;
					out[sortedPos] = in[i];
				}

				std::swap(in, out);
			}

			CBounds* boundsTmp = (CBounds*)allocTemp.allocMem(sizeof(CBounds)*nMortonCodes);
			HEntity* entitiesTmp = (HEntity*)allocTemp.allocMem(sizeof(HEntity)*nMortonCodes);
			for (uint16_t i = 0; i < nMortonCodes; i++)
			{
				boundsTmp[i] = treeBuilder.shapeBounds[i];
				entitiesTmp[i] = treeBuilder.shapeEntities[i];
			}

			std::memcpy(treeBuilder.shapeBounds.data(), boundsTmp, sizeof(CBounds)*nMortonCodes);
			std::memcpy(treeBuilder.shapeEntities.data(), entitiesTmp, sizeof(HEntity)*nMortonCodes);

			treeBuilder.buildNodes.resize(nMortonCodes*2);
			treeBuilder.buildNodesBounds.resize(nMortonCodes * 2);
			for (uint32_t iStart = 0, iEnd = 1, iNodes=0; iEnd <= nMortonCodes; iEnd++)
			{
				constexpr uint32_t mask = 0b00111111111111000000000000000000;
				if (iEnd == nMortonCodes ||
					(mortonCodes[iStart].value & mask) != (mortonCodes[iEnd].value & mask))
				{
					size_t hEntity = ecs->createEntity();

					LBVHSubTreeBuilder  buildNode;
					buildNode.iStartShape = iStart;
					buildNode.nShapes = iEnd - iStart;
					buildNode.nodes = &treeBuilder.buildNodes[iNodes];
					buildNode.nodesBounds = &treeBuilder.buildNodesBounds[iNodes];
					treeBuilder.subTreeBuilders.push_back(buildNode);

					iNodes += buildNode.nShapes * 2 - 1;
					iStart = iEnd;
				}
			}

			treeBuilder.sahBuildNodes.reserve(treeBuilder.subTreeBuilders.size() * 2 - 1);
			treeBuilder.sahBuildNodesBounds.reserve(treeBuilder.subTreeBuilders.size() * 2 - 1);
		}, treeBuilders);
	}
};

class JobEmitLBVH : public JobParallazible
{
	virtual uint32_t updateNStartThreads(uint32_t nWorkThreads) override
	{
		return nWorkThreads;
	}

	void update(WECS* ecs, uint8_t iThread) override
	{
		ComponentsGroup<CLBVHTreeBuilder> treeBuilders = queryComponentsGroup<CLBVHTreeBuilder>();
		for_each([this, ecs, iThread](CLBVHTreeBuilder& treeBuilder)
		{
			uint16_t nSubTreeBuilders = treeBuilder.subTreeBuilders.size();
			uint16_t slice = nSubTreeBuilders /getNumThreads();

			uint16_t iStart = iThread * slice;
			uint16_t iEnd = std::min(iStart + slice+1, nSubTreeBuilders);
			for (uint16_t i = iStart; i < iEnd; i++)
			{
				LBVHSubTreeBuilder* subTreeBuilder = treeBuilder.subTreeBuilders[i];

				const int firstBitIndex = 30 - 12-1;

				emitLBVH(treeBuilder, subTreeBuilder->nodes, subTreeBuilder->bounds, 
						 subTreeBuilder->iStartShape, subTreeBuilder->nShapes, firstBitIndex, subTreeBuilder->nNodes);
			}
		}, treeBuilders);
	}

	BVHBuildNode* emitLBVH(CLBVHTreeBuilder& treeBuilder, BVHBuildNode* nodes, DBounds3f* nodesBounds,
				  uint16_t iStartShape, uint16_t nShapes, int bitIndex, uint16_t& totalNodes)
	{
		if (bitIndex == -1 || nShapes < maxPrimitiveInNode)
		{
			totalNodes++;
			BVHBuildNode* node = nodes++;
			DBounds3f* bounds = nodesBounds++;
			for (uint16_t i = 0; i < nShapes; i++)
			{
				uint16_t iShape = i + iStartShape;
			    *bounds = DBounds3f(*bounds, treeBuilder.shapeBounds[iShape]);
			}
			node->initAsLeaf(iStartShape, nShapes, bounds);
		}
		else
		{ 
			uint32_t mask = 1 << bitIndex;

			if ((treeBuilder.shapeMortonCodes[iStartShape].value & mask) ==
				(treeBuilder.shapeMortonCodes[iStartShape + nShapes - 1].value & mask))
			{
				return emitLBVH(treeBuilder, nodes, nodesBounds, iStartShape, nShapes, bitIndex - 1, totalNodes);
			}
			else
			{
				uint32_t searchStart = iStartShape;
				uint32_t searchEnd = iStartShape + nShapes - 1;
				while (searchStart + 1 != searchEnd)
				{
					uint32_t mid = (searchStart + searchEnd) / 2;
					if ((treeBuilder.shapeMortonCodes[searchStart].value & mask) ==
						(treeBuilder.shapeMortonCodes[mid].value  & mask))
						searchStart = mid;
					else
						searchEnd = mid;
				}
				uint32_t splitOffset = searchEnd;
				BVHBuildNode* node = nodes++;
				DBounds3f* bounds = bounds++;
				totalNodes++;
				std::array<BVHBuildNode*, 2> childrens = {
					emitLBVH(treeBuilder, nodes, nodesBounds, iStartShape, splitOffset - iStartShape, bitIndex - 1, totalNodes),
					emitLBVH(treeBuilder, nodes, nodesBounds, splitOffset, nShapes - (splitOffset - iStartShape), bitIndex - 1, totalNodes)
				};

				int axis = bitIndex % 3;
				node->initAsInterior(bitIndex, childrens[0], childrens[1], bounds);
				return node;
			}
		}
	}

	uint32_t maxPrimitiveInNode;
};
class JobBuildUpperSAH: public Job
{

	virtual void update(WECS* ecs) override
	{
		ComponentsGroup<CLBVHTreeBuilder> treeBuilders = queryComponentsGroup<CLBVHTreeBuilder>();
		for_each([this, ecs](HEntity hEntity, CLBVHTreeBuilder& treeBuilder)
		{
			BVHBuildNode* root = buildUpperSAH(treeBuilder, 0, treeBuilder.subTreeBuilders.size()-1);
			

			CLBVHTree linearTree;
			uint16_t nNodesTotal = treeBuilder.nSAHNodesTotal + treeBuilder.nSubTreeNodesTotal;
			linearTree.nodes.resize(nNodesTotal);
			linearTree.nodesBounds.resize(nNodesTotal);
			linearTree.shapesEntities = std::move(treeBuilder.shapeEntities);

			uint16_t offset = 0;
			flattenBVHTree(root, linearTree, offset);
			
			ecs->addComponent<CLBVHTree>(hEntity, std::move(linearTree));
			ecs->removeComponent<CLBVHTreeBuilder>(hEntity);
		}, treeBuilders);
	}

	uint16_t flattenBVHTree(BVHBuildNode* node, CLBVHTree& tree, uint16_t& offset)
	{
		LinearBVHNode *lNode = &tree.nodes[offset];
		DBounds3f* lNodeBounds = &tree.nodesBounds[offset];
		*lNodeBounds = *node->bounds;

		uint16_t myOffset = offset++;
		if (node->nShapes > 0)
		{
			lNode->shapeOffset = node->iStartShape;
			lNode->nShapes = node->nShapes;
		}
		else
		{
			lNode->axis = node->splitAxis;
			lNode->nShapes = 0;
			flattenBVHTree(node->children[0], tree, offset);
			lNode->secondChildOffset =
			flattenBVHTree(node->children[1], tree, offset);
		}

		return myOffset;
	}

	BVHBuildNode* buildUpperSAH(CLBVHTreeBuilder& treeBuilder, uint16_t iStart, uint16_t iLast)
	{
		DBounds3f bounds;
		DBounds3f centroidsBounds;
		for (uint16_t i = iStart; i <= iLast; i++)
		{
			treeBuilder.nSubTreeNodesTotal += treeBuilder.subTreeBuilders[i].nNodes;
			DBounds3f* rootNodeBounds = treeBuilder.subTreeBuilders[i].nodes[0]->bounds;
			bounds = DBounds3f(bounds, *rootNodeBounds);

			DPoint3f centroid = (rootNodeBounds->pMin + rootNodeBounds->pMax)*0.5;
			centroidsBounds = DBounds3f(centroidsBounds, centroid);
		}
		uint8_t splitDim = centroidsBounds.maxExtent();
		constexpr uint8_t nBuckets = 12;


		std::array<DBounds3f, nBuckets> bucketsBounds;
		std::array<uint16_t, nBuckets> bucketsCount;
		bucketsCount.fill(0);

		for (uint16_t i = iStart; i <= iLast; i++)
		{
			DBounds3f* rootNodeBounds = treeBuilder.subTreeBuilders[i].nodes[0]->bounds;
			float centroid = (rootNodeBounds->pMin[splitDim] + rootNodeBounds->pMax[splitDim])*0.5;
			uint8_t iBucket = nBuckets * ((centroid - centroidsBounds.pMin[splitDim]) /
				(centroidsBounds.pMax[splitDim] - centroidsBounds.pMax[splitDim]));
			if (iBucket == nBuckets)
			{
				iBucket = nBuckets - 1;
			}

			treeBuilder.subTreeBuilders[i].iBucket = iBucket;

			bucketsCount[iBucket]++;
			bucketsBounds[iBucket] = DBounds3f(bucketsBounds[iBucket], *rootNodeBounds);
		}

		std::array<float, nBuckets - 1> costs;

		for (uint8_t i = 0; i < nBuckets - 1; i++)
		{
			DBounds3f b0, b1;
			uint16_t count0, count1;
			for (uint8_t j = 0; j <= i; j++)
			{
				b0 = DBounds3f(b0, bucketsBounds[j]);
				count0 += bucketsCount[j];
			}

			for (uint8_t j = i + 1; j < nBuckets; j++)
			{
				b1 = DBounds3f(b1, bucketsBounds[j]);
				count1 += bucketsCount[j];
			}

			costs[i] = 0.125 + (count0*b0.area() + count1 * b1.area()) / bounds.area();
		}

		float costMin = costs[0];
		uint8_t iBucketMinCost = 0;
		for (uint8_t i = 1; i < nBuckets - 1; i++)
		{
			if (costs[i] < costMin)
			{
				iBucketMinCost = i;
				costMin = costs[i];
			}
		}


		
		LBVHSubTreeBuilder** split = std::partition(&treeBuilder.subTreeBuilders[iStart], &treeBuilder.subTreeBuilders[iLast],
													[iBucketMinCost](const LBVHSubTreeBuilder* builder)
		{
			return builder->iBucket <= iBucketMinCost;
		});

		uint16_t mid = split - &treeBuilder.subTreeBuilders[iStart];

		DBounds3f& nodeBounds = treeBuilder.sahBuildNodesBounds.emplace_back();
		BVHBuildNode& node = treeBuilder.sahBuildNodes.emplace_back();
		treeBuilder.nSAHNodesTotal++;
		node.initAsInterior(splitDim, 
							buildUpperSAH(treeBuilder, iStart, mid - 1), 
							buildUpperSAH(treeBuilder, mid, iLast), 
							&nodeBounds);
		return &node;
	}
};


WPBR_END