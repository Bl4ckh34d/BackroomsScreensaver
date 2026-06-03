    void BuildStaticInstanceChunks(std::vector<PendingStaticInstance>& pendingStaticInstances,
                                   const std::vector<InstancedMeshRange>& instancedMeshRanges,
                                   const std::vector<uint32_t>& instancedIndices,
                                   std::vector<StaticInstanceData>& instancedInstanceData) {
        if (pendingStaticInstances.empty()) return;

        auto instanceChunkX = [](const PendingStaticInstance& instance) {
            return instance.minTileX / 4;
        };
        auto instanceChunkY = [](const PendingStaticInstance& instance) {
            return instance.minTileY / 4;
        };
        std::sort(pendingStaticInstances.begin(), pendingStaticInstances.end(),
            [&](const PendingStaticInstance& a, const PendingStaticInstance& b) {
                if (a.meshId != b.meshId) return a.meshId < b.meshId;
                if (a.castsShadow != b.castsShadow) return a.castsShadow < b.castsShadow;
                if (instanceChunkY(a) != instanceChunkY(b)) return instanceChunkY(a) < instanceChunkY(b);
                if (instanceChunkX(a) != instanceChunkX(b)) return instanceChunkX(a) < instanceChunkX(b);
                if (a.minTileY != b.minTileY) return a.minTileY < b.minTileY;
                return a.minTileX < b.minTileX;
            });

        size_t i = 0;
        while (i < pendingStaticInstances.size()) {
            const PendingStaticInstance& first = pendingStaticInstances[i];
            const int groupX = instanceChunkX(first);
            const int groupY = instanceChunkY(first);
            const bool castsShadow = first.castsShadow;
            const UINT meshId = first.meshId;
            const InstancedMeshRange& range = instancedMeshRanges[static_cast<size_t>(meshId)];

            StaticInstanceChunk chunk{};
            chunk.startIndex = range.startIndex;
            chunk.indexCount = range.indexCount;
            chunk.baseVertex = range.baseVertex;
            chunk.startInstance = static_cast<UINT>(instancedInstanceData.size());
            chunk.minTileX = first.minTileX;
            chunk.minTileY = first.minTileY;
            chunk.maxTileX = first.maxTileX;
            chunk.maxTileY = first.maxTileY;
            XMFLOAT3 minP = first.min;
            XMFLOAT3 maxP = first.max;

            size_t j = i;
            while (j < pendingStaticInstances.size()) {
                const PendingStaticInstance& instance = pendingStaticInstances[j];
                if (instance.meshId != meshId || instance.castsShadow != castsShadow ||
                    instanceChunkX(instance) != groupX || instanceChunkY(instance) != groupY) {
                    break;
                }
                instancedInstanceData.push_back(instance.data);
                chunk.minTileX = std::min(chunk.minTileX, instance.minTileX);
                chunk.minTileY = std::min(chunk.minTileY, instance.minTileY);
                chunk.maxTileX = std::max(chunk.maxTileX, instance.maxTileX);
                chunk.maxTileY = std::max(chunk.maxTileY, instance.maxTileY);
                minP.x = std::min(minP.x, instance.min.x);
                minP.y = std::min(minP.y, instance.min.y);
                minP.z = std::min(minP.z, instance.min.z);
                maxP.x = std::max(maxP.x, instance.max.x);
                maxP.y = std::max(maxP.y, instance.max.y);
                maxP.z = std::max(maxP.z, instance.max.z);
                ++j;
            }

            chunk.instanceCount = static_cast<UINT>(j - i);
            chunk.center = {(minP.x + maxP.x) * 0.5f, (minP.y + maxP.y) * 0.5f, (minP.z + maxP.z) * 0.5f};
            float dx = maxP.x - chunk.center.x;
            float dy = maxP.y - chunk.center.y;
            float dz = maxP.z - chunk.center.z;
            chunk.radius = std::sqrt(dx * dx + dy * dy + dz * dz);
            staticSceneGeometry_.instancedOpaqueChunks.push_back(chunk);
            if (castsShadow) {
                staticSceneGeometry_.instancedPropShadowChunks.push_back(chunk);
            }
            i = j;
        }
        staticSceneGeometry_.instancedIndexCount = static_cast<UINT>(instancedIndices.size());
        staticSceneGeometry_.instancedInstanceCount = static_cast<UINT>(instancedInstanceData.size());
    }
