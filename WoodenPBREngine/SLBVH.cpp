#include "pch.h"
#include "SLBVH.h"
WPBR_BEGIN
DECL_OUT_COMP_DATA(CSceneTreeLBVH)
DECL_OUT_COMP_DATA(CLBVHTreeBuilder)



void JobProcessRayCastsResults::update(WECS* ecs, uint8_t iThread) 
{
	uint32_t nRaysCasts = queryComponentsGroup<CRayCast>().size<CRayCast>();

	uint32_t slice = (nRaysCasts + getNumThreads() - 1) / getNumThreads();
	uint32_t iStart = iThread * slice;
	ComponentsGroup<CRayCast> rayCasts = queryComponentsGroup<CRayCast>();
	for_each([ecs](HEntity hEntity, CRayCast& rayCast)
	{
		HEntity interactionEntity;
		HEntity hShape;
		float tHitMin = std::numeric_limits<float>::infinity();
		for (uint32_t i = 0; i < rayCast.interactionEntities.size(); i++)
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
				hShape = interactionRequest.hShape;
			}
		}

		if (tHitMin == std::numeric_limits<float>::infinity())
		{
			CInteraction interaction;
			interaction.hCollision.h = INVALID_HANDLE;

			ecs->addComponent<CInteraction>(hEntity, std::move(interaction));
		}
		else
		{
			if (rayCast.bSurfInteraction)
			{
				CInteractionFullRequests& requests = ecs->getComponent<CInteractionFullRequests>(hShape);
				requests.push_back(interactionEntity);
			}
			else
			{
				CInteraction interaction;
				interaction.p = rayCast.ray(tHitMin);
				interaction.time = tHitMin;
				interaction.wo = -rayCast.ray.dir;
				interaction.hCollision = hShape;

				ecs->addComponent<CInteraction>(hEntity, std::move(interaction));
			}
		}
	}, rayCasts);
}

void JobProcessRayCastsResults::finish(WECS* ecs)
{
}

void JobCollisionFinish::update(WECS* ecs)
{

}

void JobCollisionFinish::finish(WECS* ecs)
{
	ecs->deleteEntitiesOfComponents<CInteractionRequest>();
	ecs->clearComponents<CInteractionRequest>();
	ecs->clearComponents<CRayCast>();
}


void JobProcessRayCasts::update(WECS* ecs, uint8_t iThread) 
{
	uint32_t nRays = queryComponentsGroup<CRayCast>().size<CRayCast>();

	uint32_t slice = (nRays+nThreads-1) /getNumThreads();
	uint32_t iStart = iThread * slice;
	ComponentsGroup<CRayCast> rays = queryComponentsGroup<CRayCast>();
	ComponentsGroup<CSceneTreeLBVH> trees = queryComponentsGroup<CSceneTreeLBVH>();
	const CSceneTreeLBVH& tree = trees.getRawData<CSceneTreeLBVH>()[0];
	for_each([tree, ecs](HEntity hEntity, CRayCast& rayCast)
	{

		const DRayf ray = rayCast.ray;
		DVector3f dirInv = DVector3f(1.0) / ray.dir;
		int dirIsNeg[3] = { dirInv.x() < 0, dirInv.y() < 0, dirInv.z() < 0 };
		int toVisitOffset = 0, currentNodeIndex = 0;
		int nodesToVisit[256];
		while (true)
		{
			const LinearBVHNode& node = tree.nodes[currentNodeIndex];
			const DBounds3f& nodeBounds = tree.nodesBounds[currentNodeIndex];
			if (nodeBounds.intersect(ray, dirInv, dirIsNeg))
			{
				if (node.nShapes > 0)
				{
					for (uint32_t i = 0; i < node.nShapes; ++i)
					{
						HEntity shapeEntity = tree.shapesEntities[node.shapeOffset + i];

						CInteractionRequest request;
						request.ray = ray;
						request.hRayCast = hEntity;
						request.hShape = shapeEntity;

						HEntity eRequest = ecs->createEntity();
						ecs->addComponent<CInteractionRequest>(eRequest, std::move(request));
						
						CInteractionRequests& requests = ecs->getComponent<CInteractionRequests>(shapeEntity);
						requests.push_back(eRequest);

						rayCast.interactionEntities.push_back(eRequest);
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

void JobGenerateShapesCentroidBound::update(WECS* ecs) 
{
	ComponentsGroup<CCentroid> centroids = queryComponentsGroup<CCentroid>();
	uint32_t nShapes = centroids.size<CCentroid>();

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




void JobGenerateShapesMortonCode::update(WECS* ecs, uint8_t iThread)
{
	ComponentsGroup<CLBVHTreeBuilder> treeBuilderData = queryComponentsGroup<CLBVHTreeBuilder>();

	Slice slice = Slice(iThread*sliceSize, sliceSize);
	ComponentsGroup<CCentroid, CBounds> shapesData = queryComponentsGroup<CCentroid, CBounds>();

	for_each([this, ecs, &shapesData, iThread](HEntity, CLBVHTreeBuilder& treeBuilder)
	{
		uint32_t iShape = iThread;
		for_each([this, ecs, &shapesData, &treeBuilder, &iShape]
		(HEntity hEntity, const CCentroid& eCentroid, const CBounds& eBounds)
		{
			constexpr int32_t mortonBits = 10;
			constexpr int32_t mortonScale = 1 << mortonBits;
			DPoint3f centroidN = treeBuilder.boundsCentroid.getLerpFactors(eCentroid);
			uint32_t mortonCode =
				(leftShift3(centroidN.z()*mortonScale) << 2) |
				(leftShift3(centroidN.y()*mortonScale) << 1) |
				leftShift3(centroidN.x()*mortonScale);

			CentroidMortonCode code;
			code.value = mortonCode;
			code.index = treeBuilder.shapeMortonCodes.size();

			treeBuilder.shapeMortonCodes.emplace_back(std::move(code));
			treeBuilder.shapeBounds.emplace_back(eBounds);
			treeBuilder.shapeEntities.emplace_back(hEntity);
			iShape++;
		}, shapesData);
	}, treeBuilderData);
}

uint32_t JobGenerateShapesMortonCode::leftShift3(uint32_t x)
{
	if (x == (1 << 10)) --x;
	x = (x | (x << 16)) & 0b00000011000000000000000011111111;
	x = (x | (x << 8)) & 0b00000011000000001111000000001111;
	x = (x | (x << 4)) & 0b00000011000011000011000011000011;
	x = (x | (x << 2)) & 0b00001001001001001001001001001001;
	return x;
}


	
void JobSortsShapesByMortonCode::update(WECS* ecs) 
{
	ComponentsGroup<CLBVHTreeBuilder> treeBuilders = queryComponentsGroup<CLBVHTreeBuilder>();
	for_each([this, ecs](HEntity, CLBVHTreeBuilder& treeBuilder)
	{
		AllocatorLinear& allocTemp = getAllocatorTemp();
		uint32_t nMortonCodes = treeBuilder.shapeEntities.size();

		
		CentroidMortonCode* mortonCodes = treeBuilder.shapeMortonCodes.data();
		if (nMortonCodes > 1)
		{
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

				std::array<uint32_t, nBuckets> counters;
				std::memset(counters.data(), 0, sizeof(uint32_t)*nBuckets);

				for (uint32_t i = 0; i < nMortonCodes; i++)
				{
					uint8_t iCounter = (in[i].value >> lowBit) & bitMask;
					++counters[iCounter];
				}


				// convert counters to offsets
				uint32_t total = 0;
				for (uint32_t i = 0; i < nBuckets; i++)
				{
					uint32_t tmp = counters[i];
					counters[i] = total;
					total += tmp;
				}

				for (uint32_t i = 0; i < nMortonCodes; i++)
				{
					uint8_t iCounter = (in[i].value >> lowBit) & bitMask;
					uint32_t sortedPos = counters[iCounter]++;
					out[sortedPos] = in[i];
				}

				std::swap(in, out);
			}

			CBounds* boundsTmp = (CBounds*)allocTemp.allocMem(sizeof(CBounds)*nMortonCodes, alignof(CBounds));
			HEntity* entitiesTmp = (HEntity*)allocTemp.allocMem(sizeof(HEntity)*nMortonCodes);
			for (uint32_t i = 0; i < nMortonCodes; i++)
			{
				boundsTmp[i] = treeBuilder.shapeBounds[mortonCodes[i].index];
				entitiesTmp[i] = treeBuilder.shapeEntities[mortonCodes[i].index];
			}

			std::memcpy(treeBuilder.shapeBounds.data(), boundsTmp, sizeof(CBounds)*nMortonCodes);
			std::memcpy(treeBuilder.shapeEntities.data(), entitiesTmp, sizeof(HEntity)*nMortonCodes);
		}

		treeBuilder.buildNodes.resize(nMortonCodes * 2);
		treeBuilder.buildNodesBounds.resize(nMortonCodes * 2);
		for (uint32_t iStart = 0, iEnd = 1, iNodes = 0; iEnd <= nMortonCodes; iEnd++)
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

				iNodes += buildNode.nShapes * 2 - 1;
				iStart = iEnd;

				treeBuilder.subTreeBuilders.emplace_back(std::move(buildNode));

			}
		}

		treeBuilder.sahBuildNodes.reserve(treeBuilder.subTreeBuilders.size() * 2 - 1);
		treeBuilder.sahBuildNodesBounds.reserve(treeBuilder.subTreeBuilders.size() * 2 - 1);
	}, treeBuilders);
}



void JobEmitLBVH::update(WECS* ecs, uint8_t iThread) 
{
	ComponentsGroup<CLBVHTreeBuilder> treeBuilders = queryComponentsGroup<CLBVHTreeBuilder>();
	for_each([this, ecs, iThread](HEntity, CLBVHTreeBuilder& treeBuilder)
	{
		uint32_t nSubTreeBuilders = treeBuilder.subTreeBuilders.size();
		uint32_t slice = nSubTreeBuilders / getNumThreads();

		uint32_t iStart = iThread * slice;
		uint32_t iEnd = min(iStart + slice + 1, nSubTreeBuilders);
		for (uint32_t i = iStart; i < iEnd; i++)
		{
			LBVHSubTreeBuilder* subTreeBuilder = &treeBuilder.subTreeBuilders[i];

			const int firstBitIndex = 30 - 12 - 1;
			subTreeBuilder->nNodes = 0;

			uint32_t boundsOffset = 0;
			emitLBVH(treeBuilder, subTreeBuilder->nodes, subTreeBuilder->nodesBounds, boundsOffset,
						subTreeBuilder->iStartShape, subTreeBuilder->nShapes, firstBitIndex, subTreeBuilder->nNodes);
			treeBuilder.nSubTreeNodesTotal += subTreeBuilder->nNodes;
		}
	}, treeBuilders);
}

BVHBuildNode* JobEmitLBVH::emitLBVH(CLBVHTreeBuilder& treeBuilder, BVHBuildNode* nodes, DBounds3f* nodesBounds, uint32_t& boundsOffset,
									uint32_t iStartShape, uint32_t nShapes, int bitIndex, uint32_t& totalNodes)
{
	maxPrimitiveInNode = 30;
	if (bitIndex == -1 || nShapes < maxPrimitiveInNode)
	{
		totalNodes++;
		BVHBuildNode* node = &nodes[boundsOffset];
		DBounds3f* bounds = &nodesBounds[boundsOffset++];
		for (uint32_t i = 0; i < nShapes; i++)
		{
			uint32_t iShape = i + iStartShape;
			*bounds = DBounds3f(*bounds, treeBuilder.shapeBounds[iShape]);
		}
		node->initAsLeaf(iStartShape, nShapes, bounds);
		return node;
	}
	else
	{
		uint32_t mask = 1 << bitIndex;

		if ((treeBuilder.shapeMortonCodes[iStartShape].value & mask) ==
			(treeBuilder.shapeMortonCodes[iStartShape + nShapes - 1].value & mask))
		{
			return emitLBVH(treeBuilder, nodes, nodesBounds, boundsOffset, iStartShape, nShapes, bitIndex - 1, totalNodes);
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
			BVHBuildNode* node = &nodes[boundsOffset];
			DBounds3f* bounds = &nodesBounds[boundsOffset++];
			totalNodes++;
			std::array<BVHBuildNode*, 2> childrens = {
				emitLBVH(treeBuilder, nodes, nodesBounds, boundsOffset, iStartShape, splitOffset - iStartShape, bitIndex - 1, totalNodes),
				emitLBVH(treeBuilder, nodes, nodesBounds, boundsOffset,splitOffset, nShapes - (splitOffset - iStartShape), bitIndex - 1, totalNodes)
			};

			int axis = bitIndex % 3;
			node->initAsInterior(axis, childrens[0], childrens[1], bounds);
			return node;
		}
	}
}



void JobBuildUpperSAH::update(WECS* ecs) 
{
	ComponentsGroup<CLBVHTreeBuilder> treeBuilders = queryComponentsGroup<CLBVHTreeBuilder>();
	for_each([this, ecs](HEntity hEntity, CLBVHTreeBuilder& treeBuilder)
	{
		BVHBuildNode* root = buildUpperSAH(treeBuilder, 0, treeBuilder.subTreeBuilders.size()-1);
			

		CSceneTreeLBVH linearTree;
		uint32_t nNodesTotal = treeBuilder.nSAHNodesTotal + treeBuilder.nSubTreeNodesTotal;
		linearTree.nodes.resize(nNodesTotal);
		linearTree.nodesBounds.resize(nNodesTotal);
		linearTree.shapesEntities = std::move(treeBuilder.shapeEntities);

		uint32_t offset = 0;
		flattenBVHTree(root, linearTree, offset);
			
		ecs->addComponent<CSceneTreeLBVH>(hEntity, std::move(linearTree));
		ecs->removeComponent<CLBVHTreeBuilder>(hEntity);
	}, treeBuilders);
}

uint32_t JobBuildUpperSAH::flattenBVHTree(BVHBuildNode* node, CSceneTreeLBVH& tree, uint32_t& offset)
{
	LinearBVHNode *lNode = &tree.nodes[offset];
	DBounds3f* lNodeBounds = &tree.nodesBounds[offset];
	*lNodeBounds = *node->bounds;

	uint32_t myOffset = offset++;
	if (node->nShapes > 0)
	{
		lNode->shapeOffset = node->iStartShape;
		lNode->nShapes = node->nShapes;
	}
	else
	{
		lNode->axis = node->splitAxis;
		assert(lNode->axis < 3);
		lNode->nShapes = 0;
		flattenBVHTree(node->children[0], tree, offset);
		lNode->secondChildOffset =
		flattenBVHTree(node->children[1], tree, offset);
	}

	return myOffset;
}

BVHBuildNode* JobBuildUpperSAH::buildUpperSAH(CLBVHTreeBuilder& treeBuilder, uint32_t iStart, uint32_t iLast)
{
	assert(iStart <= iLast);
	if (iStart == iLast)
	{
		return &treeBuilder.subTreeBuilders[iStart].nodes[0];
	}

	DBounds3f bounds;
	DBounds3f centroidsBounds;
	for (uint32_t i = iStart; i <= iLast; i++)
	{
		//treeBuilder.nSubTreeNodesTotal += treeBuilder.subTreeBuilders[i].nNodes;
		const DBounds3f& rootNodeBounds = *treeBuilder.subTreeBuilders[i].nodes[0].bounds;
		bounds = DBounds3f(bounds, rootNodeBounds);

		DPoint3f centroid = (rootNodeBounds.pMin + rootNodeBounds.pMax)*0.5;
		centroidsBounds = DBounds3f(centroidsBounds, centroid);
	}
	uint8_t splitDim = centroidsBounds.maxExtent();
	constexpr uint8_t nBuckets = 12;


	std::array<DBounds3f, nBuckets> bucketsBounds;
	std::array<uint32_t, nBuckets> bucketsCount;
	bucketsCount.fill(0);

	for (uint32_t i = iStart; i <= iLast; i++)
	{
		float dist = centroidsBounds.pMax[splitDim] - centroidsBounds.pMin[splitDim];

		DBounds3f* rootNodeBounds = treeBuilder.subTreeBuilders[i].nodes[0].bounds;
		float centroid = (rootNodeBounds->pMin[splitDim] + rootNodeBounds->pMax[splitDim])*0.5;

		float t = (centroid - centroidsBounds.pMin[splitDim]) / dist;
		uint8_t iBucket = nBuckets * t;
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
		uint32_t count0, count1;
		count0 = 0;
		count1 = 0;
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
		
		float lres = (count0 == 0) ? 0: count0 * b0.area();
		float rres = (count1 == 0) ? 0 : count1 * b1.area();
		costs[i] = 0.125 + (lres + rres) / bounds.area();
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


		
	LBVHSubTreeBuilder* split = std::partition(&treeBuilder.subTreeBuilders[iStart], &treeBuilder.subTreeBuilders[iLast]+1,
												[iBucketMinCost](const LBVHSubTreeBuilder& builder)
	{
		return builder.iBucket <= iBucketMinCost;
	});

	uint32_t mid = split - &treeBuilder.subTreeBuilders[0];

	if (mid == iLast + 1)
	{
		mid = (iLast - iStart + 1) / 2 + iStart;
	}

	DBounds3f& nodeBounds = treeBuilder.sahBuildNodesBounds.emplace_back();
	BVHBuildNode& node = treeBuilder.sahBuildNodes.emplace_back();
	treeBuilder.nSAHNodesTotal++;
	node.initAsInterior(splitDim, 
						buildUpperSAH(treeBuilder, iStart, mid - 1),
						buildUpperSAH(treeBuilder, mid, iLast), 
						&nodeBounds);
	return &node;
}
WPBR_END


