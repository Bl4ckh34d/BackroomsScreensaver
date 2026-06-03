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
