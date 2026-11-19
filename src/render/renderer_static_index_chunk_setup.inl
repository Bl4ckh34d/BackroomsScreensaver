
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
        };
        std::vector<ChunkBuild> build(static_cast<size_t>(chunksX * chunksY));
        float ox = -static_cast<float>(maze.w) * maze.tileW * 0.5f;
        float oz = -static_cast<float>(maze.h) * maze.tileD * 0.5f;
        UINT rangeEnd = std::min<UINT>(rangeStart + rangeCount, static_cast<UINT>(sourceIndices.size()));
