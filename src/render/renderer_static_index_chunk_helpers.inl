    void AppendStaticIndexChunks(const std::vector<Vertex>& vertices,
                                 const std::vector<uint32_t>& sourceIndices,
                                 UINT rangeStart,
                                 UINT rangeCount,
                                 std::vector<uint32_t>& destIndices,
                                 std::vector<StaticIndexChunk>& chunks,
                                 int chunkTiles = 4,
                                 int tilePadding = 1) const {
        GameWorldRenderSnapshot world = gameWorld_.BuildRenderSnapshot();
        if (!world.maze) return;
        const Maze& maze = *world.maze;
        const int safeChunkTiles = std::clamp(chunkTiles, 1, 16);
        const int safeTilePadding = std::max(0, tilePadding);
        int chunksX = std::max(1, (maze.w + safeChunkTiles - 1) / safeChunkTiles);
        int chunksY = std::max(1, (maze.h + safeChunkTiles - 1) / safeChunkTiles);
        struct ChunkBuild {
            std::vector<uint32_t> indices;
            XMFLOAT3 min{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
            XMFLOAT3 max{-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max()};
            int minTileX = std::numeric_limits<int>::max();
            int minTileY = std::numeric_limits<int>::max();
            int maxTileX = std::numeric_limits<int>::min();
            int maxTileY = std::numeric_limits<int>::min();
        };
        std::vector<ChunkBuild> build(static_cast<size_t>(chunksX * chunksY));
        float ox = -static_cast<float>(maze.w) * maze.tileW * 0.5f;
        float oz = -static_cast<float>(maze.h) * maze.tileD * 0.5f;
        UINT rangeEnd = std::min<UINT>(rangeStart + rangeCount, static_cast<UINT>(sourceIndices.size()));

        auto extendBounds = [](ChunkBuild& b, const XMFLOAT3& p) {
            b.min.x = std::min(b.min.x, p.x);
            b.min.y = std::min(b.min.y, p.y);
            b.min.z = std::min(b.min.z, p.z);
            b.max.x = std::max(b.max.x, p.x);
            b.max.y = std::max(b.max.y, p.y);
            b.max.z = std::max(b.max.z, p.z);
        };

        for (UINT i = rangeStart; i + 2 < rangeEnd; i += 3) {
            uint32_t ia = sourceIndices[i];
            uint32_t ib = sourceIndices[i + 1];
            uint32_t ic = sourceIndices[i + 2];
            if (ia >= vertices.size() || ib >= vertices.size() || ic >= vertices.size()) continue;
            const XMFLOAT3& a = vertices[ia].pos;
            const XMFLOAT3& b = vertices[ib].pos;
            const XMFLOAT3& c = vertices[ic].pos;
            float cx = (a.x + b.x + c.x) / 3.0f;
            float cz = (a.z + b.z + c.z) / 3.0f;
            int tileX = std::clamp(static_cast<int>(std::floor((cx - ox) / std::max(0.001f, maze.tileW))), 0, std::max(0, maze.w - 1));
            int tileY = std::clamp(static_cast<int>(std::floor((cz - oz) / std::max(0.001f, maze.tileD))), 0, std::max(0, maze.h - 1));
            int chunkX = std::clamp(tileX / safeChunkTiles, 0, chunksX - 1);
            int chunkY = std::clamp(tileY / safeChunkTiles, 0, chunksY - 1);
            ChunkBuild& chunk = build[static_cast<size_t>(chunkY * chunksX + chunkX)];
            chunk.minTileX = std::min(chunk.minTileX, tileX);
            chunk.minTileY = std::min(chunk.minTileY, tileY);
            chunk.maxTileX = std::max(chunk.maxTileX, tileX);
            chunk.maxTileY = std::max(chunk.maxTileY, tileY);
            chunk.indices.push_back(ia);
            chunk.indices.push_back(ib);
            chunk.indices.push_back(ic);
            extendBounds(chunk, a);
            extendBounds(chunk, b);
            extendBounds(chunk, c);
        }

        chunks.clear();
        chunks.reserve(build.size());
        for (ChunkBuild& b : build) {
            if (b.indices.empty()) continue;
            StaticIndexChunk chunk{};
            chunk.startIndex = static_cast<UINT>(destIndices.size());
            chunk.indexCount = static_cast<UINT>(b.indices.size());
            chunk.center = {
                (b.min.x + b.max.x) * 0.5f,
                (b.min.y + b.max.y) * 0.5f,
                (b.min.z + b.max.z) * 0.5f
            };
            float dx = b.max.x - chunk.center.x;
            float dy = b.max.y - chunk.center.y;
            float dz = b.max.z - chunk.center.z;
            chunk.radius = std::sqrt(dx * dx + dy * dy + dz * dz);
            chunk.minTileX = std::clamp(b.minTileX - safeTilePadding, 0, std::max(0, maze.w - 1));
            chunk.minTileY = std::clamp(b.minTileY - safeTilePadding, 0, std::max(0, maze.h - 1));
            chunk.maxTileX = std::clamp(b.maxTileX + safeTilePadding, 0, std::max(0, maze.w - 1));
            chunk.maxTileY = std::clamp(b.maxTileY + safeTilePadding, 0, std::max(0, maze.h - 1));
            destIndices.insert(destIndices.end(), b.indices.begin(), b.indices.end());
            chunks.push_back(chunk);
        }
    }
