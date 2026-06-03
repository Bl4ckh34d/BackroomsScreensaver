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
